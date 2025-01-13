#ifndef LLVM_STM8_MC_OBJECT_WRITER_H_INCLUDED
#define LLVM_STM8_MC_OBJECT_WRITER_H_INCLUDED

#include "llvm/MC/MCELFObjectWriter.h"

namespace llvm {

class STM8MCObjectWriter : public MCELFObjectTargetWriter {
public:
  STM8MCObjectWriter();

  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};

} // namespace llvm

#endif // LLVM_STM8_MC_OBJECT_WRITER_H_INCLUDED
