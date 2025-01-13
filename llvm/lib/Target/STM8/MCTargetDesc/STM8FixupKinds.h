#ifndef LLVM_STM8_FIXUP_KINDS_H
#define LLVM_STM8_FIXUP_KINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace STM8 {

enum Fixups {
  /// 8-bit fixup.
  fixup_8 = FirstTargetFixupKind,
  /// 16-bit fixup.
  fixup_16,
  /// PC-relative 8-bit fixup.
  fixup_8_pcrel,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};

}
} // namespace llvm

#endif // LLVM_STM8_FIXUP_KINDS_H
