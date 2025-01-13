#include "STM8CallingConv.h"
#include "MCTargetDesc/STM8MCTargetDesc.h"
#include <llvm/CodeGen/CallingConvLower.h>
#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/IR/DataLayout.h>

namespace llvm {
namespace STM8 {

namespace {

static bool CC_SdccCallV0(unsigned ValNo, MVT ValVT, MVT LocVT,
                          CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_SdccCallV1(unsigned ValNo, MVT ValVT, MVT LocVT,
                          CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_Sdcc(unsigned ValNo, MVT ValVT, MVT LocVT,
                       CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                       CCState &State);

CallingConv::ID getEffectiveCallingConv(CallingConv::ID CC) {
  switch (CC) {
  case CallingConv::C:
    return CallingConv::STM8_SDCC_v1;
  case CallingConv::Fast:
    return CallingConv::STM8_SDCC_v1;
  case CallingConv::STM8_SDCC_v0:
  case CallingConv::STM8_SDCC_v1:
    return CC;
  default:
    break;
  }

  report_fatal_error("Unsupported calling convention");
  return 0;
}

} // namespace

CCAssignFn *getCCAssignFn(CallingConv::ID CallConv) {
  CallingConv::ID EffID = getEffectiveCallingConv(CallConv);

  switch (EffID) {
  case CallingConv::STM8_SDCC_v0:
    return &CC_SdccCallV0;
  case CallingConv::STM8_SDCC_v1:
    return &CC_SdccCallV1;
  default:
    break;
  }

  return nullptr;
}

CCAssignFn *getRetCCAssignFn(CallingConv::ID CallConv) {
  CallingConv::ID EffID = getEffectiveCallingConv(CallConv);

  switch (EffID) {
  case CallingConv::STM8_SDCC_v0:
  case CallingConv::STM8_SDCC_v1:
    return &RetCC_Sdcc;
  default:
    break;
  }

  return nullptr;
}

namespace {
#include "STM8GenCallingConv.inc"
}

} // namespace STM8
} // namespace llvm
