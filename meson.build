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

avian_includes = include_directories(
    'src',
    'include',
    'classpath',
)

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
  classpath_cppflags = [
    '-DBOOT_IMAGE',
    '-DAVIAN_CLASSPATH=""',
  ]
else
  classpath_cppflags = [
    '-DBOOT_CLASSPATH="[classpathJar]"',
    '-DAVIAN_CLASSPATH="[classpathJar]"',
  ]
endif

############################################################
# type generator
############################################################

type_generator = executable('type_generator',
  sources: [
    'src/tools/type-generator/main.cpp',
    'src/system/' + build_system + '.cpp',
    'src/system/' + build_system + '/crash.cpp',
    'src/system/' + build_system + '/memory.cpp',
    'src/system/' + build_system + '/signal.cpp',
    'src/finder.cpp',
    'src/util/arg-parser.cpp'
  ],
  include_directories: avian_includes,
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

############################################################
# libavian
############################################################

compiler_sources = [
  'src/codegen/compiler.cpp',
  'src/codegen/compiler/context.cpp',
  'src/codegen/compiler/promise.cpp',
  'src/codegen/compiler/regalloc.cpp',
  'src/codegen/compiler/resource.cpp',
  'src/codegen/compiler/site.cpp',
  'src/codegen/compiler/read.cpp',
  'src/codegen/compiler/value.cpp',
  'src/codegen/compiler/frame.cpp',
  'src/codegen/compiler/ir.cpp',
  'src/codegen/compiler/event.cpp',
  'src/debug-util.cpp',
  'src/codegen/runtime.cpp',
  'src/codegen/targets.cpp',
  'src/util/fixed-allocator.cpp',
]

x86_assembler_sources = [
  'src/codegen/target/x86/assembler.cpp',
  'src/codegen/target/x86/block.cpp',
  'src/codegen/target/x86/context.cpp',
  'src/codegen/target/x86/detect.cpp',
  'src/codegen/target/x86/encode.cpp',
  'src/codegen/target/x86/fixup.cpp',
  'src/codegen/target/x86/multimethod.cpp',
  'src/codegen/target/x86/operations.cpp',
  'src/codegen/target/x86/padding.cpp',
]

arm_assembler_sources = [
  'src/codegen/target/arm/assembler.cpp',
  'src/codegen/target/arm/block.cpp',
  'src/codegen/target/arm/context.cpp',
  'src/codegen/target/arm/fixup.cpp',
  'src/codegen/target/arm/multimethod.cpp',
  'src/codegen/target/arm/operations32.cpp',
  'src/codegen/target/arm/operations64.cpp',
]

if target_arch.startswith('arm')
  assembler_sources = arm_assembler_sources
else
  assembler_sources = x86_assembler_sources
endif

vm_sources = [
  'src/system/' + target_system + '.cpp',
  'src/system/' + target_system + '/memory.cpp',
  'src/system/' + target_system + '/crash.cpp',
  'src/system/' + target_system + '/signal.cpp',
  'src/finder.cpp',
  'src/machine.cpp',
  'src/util.cpp',
  'src/heap/heap.cpp',
  'src/' + process + '.cpp',
  'src/classpath-' + classpath + '.cpp',
  'src/builtin.cpp',
  'src/jnienv.cpp',
  'src/process.cpp',
  'src/heapdump.cpp',
  'src/' + target_arch + '.S',
]

if process == 'compile'
  vm_sources += [
    'src/compile-' + target_arch + '.S',
  ]
endif

heapwalk_sources = [
  'src/heapwalk.cpp',
]

libavian = library('avian',
  sources: [
    compiler_sources,
    assembler_sources,
    vm_sources,
    heapwalk_sources,
    classpath_cpp_sources,
    generated_code,
  ],
  include_directories: [
    avian_includes,
    include_directories(
      JAVA_HOME + '/include',
      'jdk/include/' + target_platform,
    )
  ],
  cpp_args: vm_cflags + classpath_cppflags + [
    '-DAVIAN_TARGET_ARCH=AVIAN_ARCH_' + ((target_arch == 'i386') ? 'X86' : target_arch.to_upper()),
    '-DAVIAN_TARGET_FORMAT=AVIAN_FORMAT_' + target_format.to_upper(),
  ],
  c_args: target_cflags,
  dependencies: [threads, zlib])

############################################################
# binary_to_object
############################################################

binary_to_object = executable('binary_to_object',
  sources: [
    'src/tools/object-writer/elf.cpp',
    'src/tools/object-writer/mach-o.cpp',
    'src/tools/object-writer/pe.cpp',
    'src/tools/object-writer/tools.cpp',
    'src/tools/binary-to-object/main.cpp',
  ],
  include_directories: avian_includes,
  cpp_args: vm_cflags + [
    '-D__STDC_CONSTANT_MACROS',
    '-DAVIAN_TARGET_ARCH=AVIAN_ARCH_UNKNOWN',
    '-DAVIAN_TARGET_FORMAT=AVIAN_FORMAT_UNKNOWN',
  ],
  native: true)
