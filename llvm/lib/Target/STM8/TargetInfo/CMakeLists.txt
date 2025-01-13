add_llvm_component_library(LLVMSTM8Info
  STM8TargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  STM8
)

add_dependencies(LLVMSTM8Info STM8CommonTableGen)

