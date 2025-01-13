#include "STM8MCObjectWriter.h"
#include "STM8FixupKinds.h"
#include "llvm/MC/MCValue.h"

namespace llvm {

STM8MCObjectWriter::STM8MCObjectWriter()
    : MCELFObjectTargetWriter(false, ELF::ELFOSABI_NONE, ELF::EM_STM8,
                              /* TODO HasRelocationAddend = ??? */ false) {}

unsigned STM8MCObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target,
                                          const MCFixup &Fixup,
                                          bool IsPCRel) const {
  const unsigned Kind = Fixup.getTargetKind();
  if (Kind >= FirstLiteralRelocationKind) {
    return Kind - FirstLiteralRelocationKind;
  }

  MCSymbolRefExpr::VariantKind Variant = Target.getAccessVariant();

  switch (Kind) {
  case FK_Data_1: {
    switch (Variant) {
    case MCSymbolRefExpr::VK_None:
      return ELF::R_STM8_8;
    case MCSymbolRefExpr::VK_PCREL:
      return ELF::R_STM8_8_PCREL;
    default:
      llvm_unreachable("Unsupported access variant");
    }
    break;
  }
  case FK_Data_2: {
    switch (Variant) {
    case MCSymbolRefExpr::VK_None:
      return ELF::R_STM8_16;
    default:
      llvm_unreachable("Unsupported access variant");
    }
    break;
  }
  case STM8::fixup_8:
    return ELF::R_STM8_8;
  case STM8::fixup_16:
    return ELF::R_STM8_16;
  case STM8::fixup_8_pcrel:
    return ELF::R_STM8_8_PCREL;
  default:
    break;
  }

  llvm_unreachable("Unsupported fixup kind");
}

} // namespace llvm
