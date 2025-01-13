#include "STM8Subtarget.h"
#include "STM8FrameLowering.h"
#include "STM8ISelLowering.h"

#define DEBUG_TYPE "stm8-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "STM8GenSubtargetInfo.inc"

namespace llvm {

STM8Subtarget::STM8Subtarget(const Triple &TT, StringRef CPU, StringRef FS,
                             const STM8TargetMachine &TM)
    : STM8GenSubtargetInfo(TT, CPU, /* TuneCPU */ CPU, FS), TM(TM),
      TLInfo(TM, *this) {}

const STM8InstrInfo *STM8Subtarget::getInstrInfo() const { return &InstrInfo; }

const TargetLowering *STM8Subtarget::getTargetLowering() const {
  return &TLInfo;
}

const TargetFrameLowering *STM8Subtarget::getFrameLowering() const {
  return &FLInfo;
}

const TargetRegisterInfo *STM8Subtarget::getRegisterInfo() const {
  return &RegInfo;
}

} // namespace llvm
