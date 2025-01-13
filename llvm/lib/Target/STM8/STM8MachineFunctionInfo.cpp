#include "STM8MachineFunctionInfo.h"
#include "STM8RegisterInfo.h"
#include "llvm/IR/Function.h"

namespace llvm {

STM8MachineFunctionInfo::STM8MachineFunctionInfo(
    const Function &Func, const TargetSubtargetInfo *STI) {
  ((void)STI);

  Attribute Attrib = Func.getFnAttribute("interrupt");
  if (Attrib.isStringAttribute()) {
    StringRef AttrVal = Attrib.getValueAsString();

    // getAsInteger() returns false on success
    InterruptHandler = !(AttrVal.getAsInteger(0, InterruptNum));
  }
}

bool STM8MachineFunctionInfo::GetInterruptNumber(unsigned &IntNo) const {
  if (InterruptHandler) {
    IntNo = InterruptNum;
  }

  return InterruptHandler;
}

bool STM8MachineFunctionInfo::IsVariadic() const {
  return VaArgsFI.has_value();
}

void STM8MachineFunctionInfo::SetVaArgsFrameIndex(int FI) {
  assert(!VaArgsFI.has_value() && "VaArgsFI has already been set!");
  VaArgsFI = FI;
}

int STM8MachineFunctionInfo::GetVaArgsFrameIndex() const {
  assert(VaArgsFI.has_value() && "VaArgsFI has not been set yet!");
  return VaArgsFI.value();
}

} // namespace llvm
