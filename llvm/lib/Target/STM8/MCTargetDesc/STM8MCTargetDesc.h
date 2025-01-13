#ifndef LLVM_STM8_MC_TARGET_DESC_H_INCLUDED
#define LLVM_STM8_MC_TARGET_DESC_H_INCLUDED

#include "llvm/Support/DataTypes.h"

#include <memory>

#define GET_REGINFO_ENUM
#include "STM8GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "STM8GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "STM8GenSubtargetInfo.inc"

#endif // LLVM_STM8_MC_TARGET_DESC_H_INCLUDED
