add_llvm_component_library(LLVMSTM8Desc
  STM8InstPrinter.cpp
  STM8MCAsmBackend.cpp
  STM8MCAsmInfo.cpp
  STM8MCCodeEmitter.cpp
  STM8MCObjectWriter.cpp
  STM8MCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  STM8Info
  Support
  TargetParser

  ADD_TO_COMPONENT
  STM8
)

add_dependencies(LLVMSTM8Desc STM8CommonTableGen)
