#ifndef LLVM_STM8_MC_ASM_BACKEND_H_INCLUDED
#define LLVM_STM8_MC_ASM_BACKEND_H_INCLUDED

#include "llvm/MC/MCAsmBackend.h"

namespace llvm {

class MCRegisterInfo;
class MCTargetOptions;

class STM8MCAsmBackend : public MCAsmBackend {
public:
  STM8MCAsmBackend(const MCRegisterInfo &MRI, const MCTargetOptions &Options);

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

  unsigned getNumFixupKinds() const override;

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;

  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override;

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;
};

} // namespace llvm

#endif // LLVM_STM8_MC_ASM_BACKEND_H_INCLUDED
