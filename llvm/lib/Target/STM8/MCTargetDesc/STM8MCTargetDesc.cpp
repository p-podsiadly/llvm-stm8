#include "STM8MCTargetDesc.h"

#include "../TargetInfo/STM8TargetInfo.h"
#include "STM8InstPrinter.h"
#include "STM8MCAsmBackend.h"
#include "STM8MCAsmInfo.h"
#include "STM8MCCodeEmitter.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "STM8GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "STM8GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "STM8GenRegisterInfo.inc"

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSTM8TargetMC() {
  auto &theTarget = getTheSTM8Target();

  // Register the MC assembler info
  RegisterMCAsmInfo<STM8MCAsmInfo> MCAsmInfo(theTarget);

  // Register the MC instruction info
  TargetRegistry::RegisterMCInstrInfo(theTarget, []() {
    auto instrInfo = new MCInstrInfo();
    InitSTM8MCInstrInfo(instrInfo);
    return instrInfo;
  });

  // Register the MC register info
  TargetRegistry::RegisterMCRegInfo(theTarget, [](const Triple &) {
    auto regInfo = new MCRegisterInfo();
    InitSTM8MCRegisterInfo(regInfo, 0);
    return regInfo;
  });

  // Register the subtarget info
  TargetRegistry::RegisterMCSubtargetInfo(
      theTarget, [](const Triple &TT, StringRef CPU, StringRef FS) {
        return createSTM8MCSubtargetInfoImpl(TT, CPU, /* TuneCPU */ CPU, FS);
      });

  // Register the MC instruction printer
  TargetRegistry::RegisterMCInstPrinter(
      theTarget,
      [](const Triple &T, unsigned SyntaxVariant, const llvm::MCAsmInfo &MAI,
         const MCInstrInfo &MII,
         const MCRegisterInfo &MRI) -> llvm::MCInstPrinter * {
        if (SyntaxVariant == 0) {
          return new STM8InstPrinter(MAI, MII, MRI);
        }

        return nullptr;
      });

  // Register the MC code emitter
  TargetRegistry::RegisterMCCodeEmitter(
      theTarget, [](const MCInstrInfo &II, MCContext &Ctx) -> MCCodeEmitter * {
        return new STM8MCCodeEmitter(II, Ctx);
      });

  // Register the MC assembler backend
  TargetRegistry::RegisterMCAsmBackend(
      theTarget,
      [](const Target &T, const MCSubtargetInfo &STI, const MCRegisterInfo &MRI,
         const MCTargetOptions &Options) -> MCAsmBackend * {
        return new STM8MCAsmBackend(MRI, Options);
      });
}
