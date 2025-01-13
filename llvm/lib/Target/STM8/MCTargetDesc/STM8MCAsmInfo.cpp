#include "STM8MCAsmInfo.h"

namespace llvm {

STM8MCAsmInfo::STM8MCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {
    CodePointerSize = 3; // 3 bytes.
    CalleeSaveStackSlotSize = 2; // TODO copied from AVR
    IsLittleEndian = false;
    StackGrowsUp = false;
    MaxInstLength = 5; // Instruction are at most 5 bytes long.
    SeparatorString = nullptr; // No separator for multiple instructions on the same line.
    CommentString = ";"; // Like in SDCC.
    ExceptionsType = ExceptionHandling::None;
    // TODO rest of properties.
}

}
