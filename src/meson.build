############################################################
# avian/src
############################################################

type_generator_sources = files(
  'tools/type-generator/main.cpp',
  'system/' + build_system + '.cpp',
  'system/' + build_system + '/crash.cpp',
  'system/' + build_system + '/memory.cpp',
  'system/' + build_system + '/signal.cpp',
  'finder.cpp',
  'util/arg-parser.cpp'
)

compiler_sources = files(
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
)

x86_assembler_sources = files(
  'codegen/target/x86/assembler.cpp',
  'codegen/target/x86/block.cpp',
  'codegen/target/x86/context.cpp',
  'codegen/target/x86/detect.cpp',
  'codegen/target/x86/encode.cpp',
  'codegen/target/x86/fixup.cpp',
  'codegen/target/x86/multimethod.cpp',
  'codegen/target/x86/operations.cpp',
  'codegen/target/x86/padding.cpp',
)

arm_assembler_sources = files(
  'codegen/target/arm/assembler.cpp',
  'codegen/target/arm/block.cpp',
  'codegen/target/arm/context.cpp',
  'codegen/target/arm/fixup.cpp',
  'codegen/target/arm/multimethod.cpp',
  'codegen/target/arm/operations32.cpp',
  'codegen/target/arm/operations64.cpp',
)

if target_arch.startswith('arm')
  assembler_sources = arm_assembler_sources
else
  assembler_sources = x86_assembler_sources
endif

vm_sources = files(
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
)

if process == 'compile'
  vm_sources += files(
    'compile-' + target_arch + '.S',
  )
endif

heapwalk_sources = files(
  'heapwalk.cpp',
)

binary_to_object_sources = files(
  'tools/object-writer/elf.cpp',
  'tools/object-writer/mach-o.cpp',
  'tools/object-writer/pe.cpp',
  'tools/object-writer/tools.cpp',
  'tools/binary-to-object/main.cpp',
)
