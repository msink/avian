############################################################
# avian
############################################################

project('avian', ['cpp', 'c'],
  version: '1.3.0-SNAPSHOT', 
  default_options: [
    'cpp_std=c++11',
    'buildtype=release',
    'warning_level=2',
    'default_library=static',
  ]
)

############################################################
# user defined build options
############################################################

target_platform = get_option('platform')       # {linux,windows,macosx,ios,freebsd}
target_arch     = get_option('arch')           # {i386,x86_64,arm,arm64}
process         = get_option('process')        # {compile,interpret}
mode            = get_option('mode')           # {debug,debug-fast,fast,small}
lzma            = get_option('lzma')           # <lzma source directory>
armv6           = get_option('armv6')          # {true,false}
bootimage       = get_option('bootimage')      # {true,false}
tails           = get_option('tails')          # {true,false}
continuations   = get_option('continuations')  # {true,false}
openjdk         = get_option('openjdk')        # <openjdk installation directory>
openjdk_src     = get_option('openjdk_src')    # <openjdk source directory>
android         = get_option('android')        # <android source directory>

assert(openjdk == '' or android == '', 'android and openjdk are incompatible')

if target_platform == ''
  target_platform = target_machine.system()
  if target_platform == 'darwin'
    target_platform = 'macosx'
  endif
endif

if target_arch == ''
  target_arch = target_machine.cpu_family()
  if target_arch == 'x86'
    target_arch = 'i386'
  endif
endif

############################################################
# dependencies
############################################################

threads = dependency('threads')
zlib = dependency('zlib', version: '>=1.2.3', static: true)
zlib_native = dependency('zlib', version: '>=1.2.3', static: true, native: true)

python = find_program('python3', required: false)
if not python.found()
  python = find_program('python')
endif
javac = find_program('javac')
jar = find_program('jar')

JAVA_HOME = run_command(python, '-c', 'import os; print(os.environ["JAVA_HOME"])').stdout().strip()
JAVA_VERSION = run_command(javac, '-version').stderr().split('.').get(1)

############################################################
# global variables
############################################################

if target_platform == 'windows'
  target_system = 'windows'
  target_format = 'pe'
  embed_prefix  = 'c:/avian-embedded'
elif target_platform == 'macosx' or target_platform == 'ios'
  target_system = 'posix'
  target_format = 'macho'
  embed_prefix  = '/avian-embedded'
else
  target_system = 'posix'
  target_format = 'elf'
  embed_prefix  = '/avian-embedded'
endif

build_platform = build_machine.system()
build_arch     = build_machine.cpu_family()
if build_platform == 'windows'
  build_system = 'windows'
else
  build_system = 'posix'
endif

if openjdk != ''
  classpath = 'openjdk'
elif android != ''
  classpath = 'android'
else
  classpath = 'avian'
endif

pointer_size = meson.get_compiler('cpp').sizeof('void*')

target_cflags = [
  '-DTARGET_BYTES_PER_WORD=@0@'.format(pointer_size),
]

common_cflags = [
# note that we suppress the non-virtual-dtor warning because we never
# use the delete operator, which means we don't need virtual
# destructors:
  '-Wno-non-virtual-dtor',
  '-Wunused-parameter',
  '-Winit-self',
  '-fno-rtti',
  '-fno-exceptions',
]

vm_cflags = common_cflags + target_cflags + [
  '-D__STDC_LIMIT_MACROS',
  '-D_JNI_IMPLEMENTATION_',
  '-DAVIAN_VERSION="' + meson.project_version() + '"',
  '-DAVIAN_INFO=""',
  '-DUSE_ATOMIC_OPERATIONS',
  '-DAVIAN_JAVA_HOME=""',
  '-DAVIAN_EMBED_PREFIX="' + embed_prefix + '"',
]

avian_cppflags = vm_cflags + [
  '-DAVIAN_TARGET_ARCH=AVIAN_ARCH_' + ((target_arch == 'i386') ? 'X86' : target_arch.to_upper()),
  '-DAVIAN_TARGET_FORMAT=AVIAN_FORMAT_' + target_format.to_upper(),
]

avian_includes = include_directories(
  'src',
  'include',
  'classpath',
  JAVA_HOME + '/include',
  'src/jdk/include/' + target_platform,
)

avian_lflags = [
  '-lm',
  '-lz',
  '-lws2_32',
  '-lversion',
  '-luuid',
  '-liphlpapi',
  '-lmswsock',
]

############################################################
# classpath
############################################################

subdir('classpath')

classpath_classes = custom_target('classpath_classes',
  input: classpath_java_sources,
  output: 'classpath_classes',
  capture: true,
  command: [javac,
    '-encoding', 'UTF-8',
    '-source', '1.6', # JAVA_VERSION,
    '-target', '1.6', # JAVA_VERSION,
    '-d', meson.current_build_dir() + '/classpath',
    '-bootclasspath', meson.current_build_dir() + '/classpath',
    '@INPUT@'
  ],
)

classpath_jar = custom_target('classpath_jar',
  input: classpath_classes,
  output: 'classpath.jar',
  command: [jar,
    'c0f', '@OUTPUT@',
    '-C', meson.current_build_dir() + '/classpath', '.'
  ],
)

if bootimage == true
  avian_cppflags += [
    '-DBOOT_IMAGE',
    '-DAVIAN_CLASSPATH=""',
  ]
else
  avian_cppflags += [
    '-DBOOT_CLASSPATH="[classpathJar]"',
    '-DAVIAN_CLASSPATH="[classpathJar]"',
  ]
endif

subdir('src')
subdir('test')
