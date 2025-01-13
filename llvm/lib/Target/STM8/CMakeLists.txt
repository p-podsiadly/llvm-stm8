add_llvm_component_group(STM8)

set(LLVM_TARGET_DEFINITIONS STM8.td)

tablegen(LLVM STM8GenRegisterInfo.inc      -gen-register-info)
tablegen(LLVM STM8GenRegisterBank.inc      -gen-register-bank)
tablegen(LLVM STM8GenInstrInfo.inc         -gen-instr-info)
tablegen(LLVM STM8GenSubtargetInfo.inc     -gen-subtarget)
tablegen(LLVM STM8GenMCCodeEmitter.inc     -gen-emitter)
tablegen(LLVM STM8GenMCPseudoLowering.inc  -gen-pseudo-lowering)
tablegen(LLVM STM8GenDAGISel.inc           -gen-dag-isel)
tablegen(LLVM STM8GenCallingConv.inc       -gen-callingconv)
tablegen(LLVM STM8GenAsmWriter.inc         -gen-asm-writer)
tablegen(LLVM STM8GenAsmMatcher.inc        -gen-asm-matcher)
tablegen(LLVM STM8GenDisassemblerTable.inc -gen-disassembler)

add_public_tablegen_target(STM8CommonTableGen)

add_llvm_target(STM8CodeGen
  STM8AsmPrinter.cpp
  STM8CallingConv.cpp
  STM8DAGToDAGISel.cpp
  STM8FrameLowering.cpp
  STM8InstrInfo.cpp
  STM8ISelLowering.cpp
  STM8MachineFunctionInfo.cpp
  STM8MCInstExpansion.cpp
  STM8RegisterInfo.cpp
  STM8Subtarget.cpp
  STM8TargetMachine.cpp

  Passes/STM8LateInstrElimination.cpp
  Passes/STM8EarlyImagRegLowering.cpp
  Passes/STM8LowerGenLdSt.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  MC
  SelectionDAG
  Support
  Target
  STM8Desc

  ADD_TO_COMPONENT
  STM8
)

add_subdirectory(AsmParser)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
add_subdirectory(Disassembler)
