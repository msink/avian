############################################################
# avian/src
############################################################

type_generator = executable('type_generator',
  sources: [
    'tools/type-generator/main.cpp',
    'system/' + build_system + '.cpp',
    'system/' + build_system + '/crash.cpp',
    'system/' + build_system + '/memory.cpp',
    'system/' + build_system + '/signal.cpp',
    'finder.cpp',
    'util/arg-parser.cpp'
  ],
  include_directories: include_directories('../include'),
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
    input:  'types.def',
    output: 'type-' + type + '.h',
    command: [type_generator, '-t', type,
      '-cp', meson.current_build_dir() + '/../classpath',
      '-i', '@INPUT@', '-o', '@OUTPUT@'],
    depends: classpath_classes
  )
endforeach

compiler_sources = [
  'codegen/compiler.cpp',
  'codegen/compiler/context.cpp',
  'codegen/compiler/promise.cpp',
  'codegen/compiler/regalloc.cpp',
  'codegen/compiler/resource.cpp',
  'codegen/compiler/site.cpp',
  'codegen/compiler/read.cpp',
  'codegen/compiler/value.cpp',
  'codegen/compiler/frame.cpp',
  'codegen/compiler/ir.cpp',
  'codegen/compiler/event.cpp',
  'debug-util.cpp',
  'codegen/runtime.cpp',
  'codegen/targets.cpp',
  'util/fixed-allocator.cpp',
]

x86_assembler_sources = [
  'codegen/target/x86/assembler.cpp',
  'codegen/target/x86/block.cpp',
  'codegen/target/x86/context.cpp',
  'codegen/target/x86/detect.cpp',
  'codegen/target/x86/encode.cpp',
  'codegen/target/x86/fixup.cpp',
  'codegen/target/x86/multimethod.cpp',
  'codegen/target/x86/operations.cpp',
  'codegen/target/x86/padding.cpp',
]

arm_assembler_sources = [
  'codegen/target/arm/assembler.cpp',
  'codegen/target/arm/block.cpp',
  'codegen/target/arm/context.cpp',
  'codegen/target/arm/fixup.cpp',
  'codegen/target/arm/multimethod.cpp',
  'codegen/target/arm/operations32.cpp',
  'codegen/target/arm/operations64.cpp',
]

if target_arch.startswith('arm')
  assembler_sources = arm_assembler_sources
else
  assembler_sources = x86_assembler_sources
endif

vm_sources = [
  'system/' + target_system + '.cpp',
  'system/' + target_system + '/memory.cpp',
  'system/' + target_system + '/crash.cpp',
  'system/' + target_system + '/signal.cpp',
  'finder.cpp',
  'machine.cpp',
  'util.cpp',
  'heap/heap.cpp',
  process + '.cpp',
  'classpath-' + classpath + '.cpp',
  'builtin.cpp',
  'jnienv.cpp',
  'process.cpp',
  'heapdump.cpp',
  target_arch + '.S',
]

if process == 'compile'
  vm_sources += [
    'compile-' + target_arch + '.S',
  ]
endif

heapwalk_sources = [
  'heapwalk.cpp',
]

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
  sources: [
    'tools/object-writer/elf.cpp',
    'tools/object-writer/mach-o.cpp',
    'tools/object-writer/pe.cpp',
    'tools/object-writer/tools.cpp',
    'tools/binary-to-object/main.cpp',
  ],
  include_directories: include_directories('../include', '../classpath'),
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
    'main.cpp',
    'boot.cpp',
    classpath_jar_o,
  ],
  objects: [libavian.extract_all_objects()],
  include_directories: avian_includes,
  cpp_args: avian_cppflags,
  link_args: avian_lflags,
)

jvm = shared_library('jvm',
  sources: [
    'boot.cpp',
    classpath_jar_o,
  ],
  objects: [libavian.extract_all_objects()],
  include_directories: avian_includes,
  cpp_args: avian_cppflags,
  link_args: avian_lflags,
)
ttt = '''
avian_dynamic = executable('avian-dynamic',
  sources: [
    'main.cpp',
  ],
  include_directories: avian_includes,
  cpp_args: avian_cppflags + [
    '-DBOOT_LIBRARY="' + so_prefix + 'jvm' + so_suffix + '"'
  ],
  link_args: avian_lflags,
)
'''
embed_loader = executable('embed_loader',
  sources: [
    'embedded-loader.cpp',
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
    'embed.cpp',
    embed_loader_o,
  ],
  include_directories: avian_includes,
  cpp_args: avian_cppflags,
  link_args: avian_lflags,
)
