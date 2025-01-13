add_llvm_component_library(LLVMSTM8Disassembler
  STM8Disassembler.cpp

  LINK_COMPONENTS
  STM8Info
  CodeGenTypes
  MC
  MCDisassembler
  Support

  ADD_TO_COMPONENT
  STM8
  )

