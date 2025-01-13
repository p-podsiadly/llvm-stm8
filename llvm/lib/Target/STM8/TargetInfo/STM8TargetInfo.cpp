#include "STM8TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

namespace llvm {

Target &getTheSTM8Target() {
  static Target TheSTM8Target;
  return TheSTM8Target;
}

} // namespace llvm

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSTM8TargetInfo() {
  llvm::RegisterTarget<llvm::Triple::stm8> X(llvm::getTheSTM8Target(), "stm8",
                                            "STM8 microcontroller", "STM8");
}
