add_llvm_component_library(LLVMSTM8AsmParser
  STM8AsmParser.cpp
  STM8Operand.cpp

  LINK_COMPONENTS
  STM8Desc
  STM8Info
  CodeGenTypes
  MC
  MCParser
  Support

  ADD_TO_COMPONENT
  STM8
  )
