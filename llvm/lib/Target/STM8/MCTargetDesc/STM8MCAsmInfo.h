#ifndef LLVM_STM8_MC_ASM_INFO_H
#define LLVM_STM8_MC_ASM_INFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {

class Triple;

/** Defines various properties of STM8 assembler. */
class STM8MCAsmInfo : public MCAsmInfo {
public:
    explicit STM8MCAsmInfo(const Triple &TT, const MCTargetOptions &Options);
};

}

#endif // LLVM_STM8_MC_ASM_INFO_H
