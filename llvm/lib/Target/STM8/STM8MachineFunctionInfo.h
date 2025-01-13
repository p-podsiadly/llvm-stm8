#ifndef LLVM_STM8_MACHINE_FUNCTION_INFO_H_INCLUDED
#define LLVM_STM8_MACHINE_FUNCTION_INFO_H_INCLUDED

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

class STM8MachineFunctionInfo : public MachineFunctionInfo {
public:
  /// @brief Creates STM8-specific function info. Can be accessed by calling
  /// MachineFunction::getInfo().
  /// @param Func
  /// @param STI Subtarget info is not used, but the parameter is required by
  /// MachineFunctionInfo::create().
  STM8MachineFunctionInfo(const Function &Func, const TargetSubtargetInfo *STI);

  bool IsInterruptHandler() const { return InterruptHandler; }
  bool GetInterruptNumber(unsigned &IntNo) const;

  bool IsVariadic() const;
  void SetVaArgsFrameIndex(int FI);
  int GetVaArgsFrameIndex() const;

private:
  bool InterruptHandler = false;
  unsigned InterruptNum = 0;
  std::optional<int> VaArgsFI;
};

} // namespace llvm

#endif // LLVM_STM8_MACHINE_FUNCTION_INFO_H_INCLUDED
