#include "STM8MCAsmBackend.h"
#include "STM8FixupKinds.h"
#include "STM8MCObjectWriter.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/Support/Endian.h"

namespace llvm {

STM8MCAsmBackend::STM8MCAsmBackend(const MCRegisterInfo &MRI,
                                   const MCTargetOptions &Options)
    : MCAsmBackend(endianness::big) {}

std::unique_ptr<MCObjectTargetWriter>
STM8MCAsmBackend::createObjectTargetWriter() const {
  return std::make_unique<STM8MCObjectWriter>();
}

unsigned STM8MCAsmBackend::getNumFixupKinds() const {
  return STM8::NumTargetFixupKinds;
}

const MCFixupKindInfo &
STM8MCAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[STM8::NumTargetFixupKinds] = {
      // clang-format off
      // Name            Offset  Size  Flags
      {"fixup_8",         0,      8,    0},
      {"fixup_16",        0,      16,   0},
      {"fixup_8_pcrel",   0,      8,    MCFixupKindInfo::FKF_IsPCRel},
      // clang-format on
  };

  if ((Kind < FirstTargetFixupKind) || (Kind >= FirstLiteralRelocationKind)) {
    return MCAsmBackend::getFixupKindInfo(Kind);
  }

  unsigned InfoIdx = static_cast<unsigned>(Kind - FirstTargetFixupKind);
  assert(InfoIdx < std::size(Infos));

  return Infos[InfoIdx];
}

void STM8MCAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                  const MCValue &Target,
                                  MutableArrayRef<char> Data, uint64_t Value,
                                  bool IsResolved,
                                  const MCSubtargetInfo *STI) const {
  if (Fixup.getKind() >= FirstLiteralRelocationKind) {
    return;
  }

  uint32_t BitSize = getFixupKindInfo(Fixup.getKind()).TargetSize;
  assert(BitSize % 8 == 0);
  uint32_t ByteSize = BitSize / 8;

  uint32_t Offset = Fixup.getOffset();

  if ((Offset + ByteSize) > Data.size()) {
    Asm.getContext().reportError(Fixup.getLoc(), "cannot apply fixup");
    return;
  }

  if (Fixup.getTargetKind() == STM8::fixup_8_pcrel) {
    uint8_t Addend = static_cast<uint8_t>(Data[Offset]);
    if (Addend > Value) {
      Asm.getContext().reportError(
          Fixup.getLoc(),
          "illegal PC-relative fixup value, must be >= " + utostr(Addend));
      return;
    }

    Data[Offset] = static_cast<char>(Value - Addend);
  } else if (ByteSize == 1) {
    Data[Offset] = static_cast<char>(Value & 0xFFu);
  } else if (ByteSize == 2) {
    support::endian::write16be(&Data[Offset], static_cast<uint16_t>(Value));
  } else {
    Asm.getContext().reportError(Fixup.getLoc(), "unsupported fixup of size " +
                                                     utostr(BitSize) + " bits");
  }
}

bool STM8MCAsmBackend::fixupNeedsRelaxation(const MCFixup &Fixup,
                                            uint64_t Value,
                                            const MCRelaxableFragment *DF,
                                            const MCAsmLayout &Layout) const {
  llvm_unreachable("Not implemented yet!");
}

bool STM8MCAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                    const MCSubtargetInfo *STI) const {
  const unsigned char nop = 0x9D;
  for (uint64_t byteIdx = 0; byteIdx < Count; ++byteIdx) {
    OS.write(nop);
  }

  return true;
}

} // namespace llvm
