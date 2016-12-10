############################################################
# avian
############################################################

project('avian', ['cpp', 'c'],
  version: '1.3.0-SNAPSHOT', 
  default_options: [
    'cpp_std=c++11',
    'warning_level=2',
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
  elif target_arch == 'arm' and target_machine.cpu() == 'aarch64'
    target_arch = 'arm64'
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
  so_prefix     = ''
  so_suffix     = '.dll'
  embed_prefix  = 'c:/avian-embedded'
elif target_platform == 'macosx' or target_platform == 'ios'
  target_system = 'posix'
  target_format = 'macho'
  so_prefix     = 'lib'
  so_suffix     = '.dylib'
  embed_prefix  = '/avian-embedded'
else
  target_system = 'posix'
  target_format = 'elf'
  so_prefix     = 'lib'
  so_suffix     = '.so'
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
  output: '.classpath',
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

############################################################
# src
############################################################

subdir('src')

type_generator = executable('type_generator',
  sources: type_generator_sources,
  include_directories: include_directories('src', 'include'),
  cpp_args: [ '-DPOINTER_SIZE=@0@'.format(pointer_size) ] + vm_cflags,
  c_args: target_cflags,
  dependencies: [threads, zlib_native],
  native: true)

generated_code = []
foreach type: [
    'enums',
    'declarations',
    'constructors',
    'initializations',
    'java-initializations',
    'name-initializations',
    'maps',
  ]
  generated_code += custom_target('type-' + type,
    input:  'src/types.def',
    output: 'type-' + type + '.h',
    command: [type_generator, '-t', type,
      '-cp', meson.current_build_dir() + '/classpath',
      '-i', '@INPUT@', '-o', '@OUTPUT@'],
    depends: classpath_classes
  )
endforeach

libavian = static_library('avian',
  sources: [
    compiler_sources,
    assembler_sources,
    vm_sources,
    heapwalk_sources,
    classpath_cpp_sources,
    generated_code,
  ],
  include_directories: avian_includes,
  cpp_args: avian_cppflags,
  c_args: target_cflags,
  dependencies: [threads, zlib])

binary_to_object = executable('binary_to_object',
  sources: binary_to_object_sources,
  include_directories: include_directories('src', 'include', 'classpath'),
  cpp_args: vm_cflags + [
    '-D__STDC_CONSTANT_MACROS',
    '-DAVIAN_TARGET_ARCH=AVIAN_ARCH_UNKNOWN',
    '-DAVIAN_TARGET_FORMAT=AVIAN_FORMAT_UNKNOWN',
  ],
  native: true)

classpath_jar_o = custom_target('classpath-jar.o',
  input: classpath_jar,
  output: 'classpath-jar.o',
  command: [binary_to_object,
    '@INPUT@',
    '@OUTPUT@',
    '_binary_classpath_jar_start',
    '_binary_classpath_jar_end',
    target_format,
    target_arch,
  ],
)

avian = executable('avian',
  sources: [
    'src/main.cpp',
    'src/boot.cpp',
    classpath_jar_o,
  ],
  objects: [libavian.extract_all_objects()],
  include_directories: avian_includes,
  cpp_args: avian_cppflags,
  link_args: avian_lflags,
)

jvm = shared_library('jvm',
  sources: [
    'src/boot.cpp',
    classpath_jar_o,
  ],
  objects: [libavian.extract_all_objects()],
  include_directories: avian_includes,
  cpp_args: avian_cppflags,
  link_args: avian_lflags,
)

avian_dynamic = executable('avian-dynamic',
  sources: [
    'src/main.cpp',
  ],
  include_directories: avian_includes,
  cpp_args: avian_cppflags + [
    '-DBOOT_LIBRARY="' + so_prefix + 'jvm' + so_suffix + '"'
  ],
  link_with: jvm,
  link_args: avian_lflags,
)

embed_loader = executable('embed_loader',
  sources: [
    'src/embedded-loader.cpp',
  ],
  objects: [libavian.extract_all_objects()],
  include_directories: avian_includes,
  cpp_args: avian_cppflags,
  link_args: avian_lflags,
)

embed_loader_o = custom_target('embed-loader.o',
  input: embed_loader,
  output: 'embed-loader.o',
  command: [binary_to_object,
    '@INPUT@',
    '@OUTPUT@',
    '_binary_loader_start',
    '_binary_loader_end',
    target_format,
    target_arch,
  ],
)

embed = executable('embed',
  sources: [
    'src/embed.cpp',
    embed_loader_o,
  ],
  include_directories: avian_includes,
  cpp_args: avian_cppflags,
  link_args: avian_lflags + ['-static']
)

############################################################
# test
############################################################

subdir('test')

test_jni = shared_library('test_jni',
  sources: test_jni_sources,
  include_directories: avian_includes,
  cpp_args: avian_cppflags,
  link_args: avian_lflags,
)

test_java = custom_target('test_java',
  input: test_java_sources,
  output: '.test_java',
  capture: true,
  command: [javac,
    '-encoding', 'UTF-8',
    '-source', '1.' + JAVA_VERSION,
    '-target', '1.' + JAVA_VERSION,
    '-classpath', meson.current_build_dir() + '/test',
    '-d', meson.current_build_dir() + '/test',
    '-bootclasspath', meson.current_build_dir() + '/classpath',
    '@INPUT@'
  ],
)

test_java_2 = custom_target('test_java_2',
  input: test_java_2_sources,
  output: '.test_java_2',
  capture: true,
  command: [javac,
    '-encoding', 'UTF-8',
    '-source', '1.2',
    '-target', '1.1',
    '-XDjsrlimit=0',
    '-d', meson.current_build_dir() + '/test',
    '-bootclasspath', meson.current_build_dir() + '/classpath',
    '@INPUT@'
  ],
)

test_java_extra = custom_target('test_java_extra',
  input: test_java_extra_sources,
  output: '.test_java_extra',
  capture: true,
  command: [javac,
    '-encoding', 'UTF-8',
    '-source', '1.' + JAVA_VERSION,
    '-target', '1.' + JAVA_VERSION,
    '-d', meson.current_build_dir() + '/test',
    '-bootclasspath', meson.current_build_dir() + '/classpath',
    '@INPUT@'
  ],
)
