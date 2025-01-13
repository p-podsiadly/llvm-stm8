#ifndef LLVM_LIB_TARGET_STM8_STM8TARGETMACHINE_H
#define LLVM_LIB_TARGET_STM8_STM8TARGETMACHINE_H

#include "STM8Subtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class STM8TargetMachine : public LLVMTargetMachine {
public:
  STM8TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                    bool JIT);
  ~STM8TargetMachine() override;

  TargetLoweringObjectFile *getObjFileLowering() const override;

  const STM8Subtarget *getSubtargetImpl(const Function &) const override;

  TargetPassConfig *createPassConfig(PassManagerBase &passMgr) override;

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

private:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  STM8Subtarget SubTarget;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_STM8_STM8TARGETMACHINE_H
