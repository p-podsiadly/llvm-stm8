#include "STM8AsmPrinter.h"
#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "STM8ISelLowering.h"
#include "STM8MCInstExpansion.h"
#include "STM8MachineFunctionInfo.h"
#include "TargetInfo/STM8TargetInfo.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"

#define DEBUG_TYPE "stm8-asm-printer"

namespace llvm {

STM8AsmPrinter::STM8AsmPrinter(TargetMachine &TM,
                               std::unique_ptr<MCStreamer> Streamer)
    : AsmPrinter(TM, std::move(Streamer)) {}

StringRef STM8AsmPrinter::getPassName() const {
  return "STM8 Assembly Printer";
}

void STM8AsmPrinter::emitInstruction(const MachineInstr *MI) {
  if (MI->isBundled()) {
    LLVM_DEBUG(dbgs() << " Iterating over MIs in bundle\n");
    auto MBB = MI->getParent();
    for (auto MII = MI->getIterator();
         (MII != MBB->instr_end()) && MII->isBundled(); ++MII) {
      emitUnbundledInstruction(&*MII);
    }
    LLVM_DEBUG(dbgs() << " End of a bundle\n");
  } else {
    emitUnbundledInstruction(MI);
  }
}

bool STM8AsmPrinter::runOnMachineFunction(MachineFunction &MF) {
  auto *MFInfo = MF.getInfo<STM8MachineFunctionInfo>();
  unsigned InterruptNum = 0;
  if (MFInfo->GetInterruptNumber(InterruptNum)) {
    emitISRSection(InterruptNum, MF);
  }

  return AsmPrinter::runOnMachineFunction(MF);
}

void STM8AsmPrinter::emitISRSection(int IVIndex, MachineFunction &MF) {
  MCContext &Ctx = OutStreamer->getContext();

  MCSection *CurrentSection = OutStreamer->getCurrentSectionOnly();

  MCSection *IsrSection =
      Ctx.getELFSection(".isr" + llvm::utostr(IVIndex), ELF::SHT_PROGBITS,
                        ELF::SHF_ALLOC | ELF::SHF_EXECINSTR);

  OutStreamer->switchSection(IsrSection);

  MCSymbol *Symbol = getSymbol(&MF.getFunction());
  const MCExpr *Expr = MCSymbolRefExpr::create(Symbol, Ctx);

  MCInst Instr;
  Instr.setOpcode(STM8::INTm16);
  Instr.addOperand(MCOperand::createExpr(Expr));
  OutStreamer->emitInstruction(Instr, MF.getSubtarget());
  OutStreamer->doFinalizationAtSectionEnd(IsrSection);

  OutStreamer->switchSection(CurrentSection);
}

void STM8AsmPrinter::emitUnbundledInstruction(const MachineInstr *MI) {
  MCInst Inst;
  Inst.setOpcode(MI->getOpcode());

  for (unsigned OperandIdx = 0; OperandIdx < MI->getNumOperands();
       ++OperandIdx) {
    MCOperand MCO;
    if (lowerOperand(MI->getOperand(OperandIdx), MCO)) {
      Inst.addOperand(MCO);
    }
  }

  if (MI->isPseudo()) {
    emitPseudoInst(Inst);
  } else {
    EmitToStreamer(*OutStreamer, Inst);
  }
}

bool STM8AsmPrinter::lowerOperand(const MachineOperand &MO,
                                  MCOperand &MCO) const {
  auto MOType = MO.getType();
  switch (MOType) {
  case MachineOperand::MO_Register: {
    if (MO.isImplicit()) {
      return false;
    }

    MCO = MCOperand::createReg(MO.getReg());
    return true;
  }
  case MachineOperand::MO_Immediate: {
    MCO = MCOperand::createImm(MO.getImm());
    return true;
  }
  case MachineOperand::MO_MachineBasicBlock: {
    MachineBasicBlock *MBB = MO.getMBB();
    MCSymbol *MBBSym = MBB->getSymbol();
    MCO = MCOperand::createExpr(MCSymbolRefExpr::create(MBBSym, OutContext));
    return true;
  }
  case MachineOperand::MO_GlobalAddress: {
    MCSymbol *Sym = getSymbol(MO.getGlobal());
    return lowerSymbol(Sym, MCO);
  }
  case MachineOperand::MO_ExternalSymbol: {
    MCSymbol *Sym = GetExternalSymbolSymbol(MO.getSymbolName());
    return lowerSymbol(Sym, MCO);
  }
  case MachineOperand::MO_JumpTableIndex: {
    MCSymbol *Sym = GetJTISymbol(MO.getIndex());
    return lowerSymbol(Sym, MCO);
  }
  case MachineOperand::MO_RegisterMask: {
    return false;
  }
  default:
    break;
  }

  llvm_unreachable("Not implemented yet!");
}

bool STM8AsmPrinter::lowerSymbol(const MCSymbol *Sym, MCOperand &MCO) const {
  MCO = MCOperand::createExpr(MCSymbolRefExpr::create(Sym, OutContext));
  return true;
}

void STM8AsmPrinter::emitPseudoInst(MCInst &Inst) {
  auto Expansion = ExpandSTM8Inst(Inst);

#ifndef NDEBUG
  LLVM_DEBUG(dbgs() << "  Expanding pseudo-instruction "
                    << getInstName(Inst.getOpcode()));
  for (const MCOperand &Op : Inst) {
    LLVM_DEBUG(dbgs() << " " << Op);
  }

  LLVM_DEBUG(dbgs() << "\n");
#endif

  for (const auto &EI : Expansion) {
    LLVM_DEBUG(dbgs() << "   * " << getInstName(EI.getOpcode()) << "\n");
    EmitToStreamer(*OutStreamer, EI);
  }

  if (Expansion.empty()) {
    LLVM_DEBUG(dbgs() << "    No known expansion.\n");
    auto *TII = MF->getSubtarget().getInstrInfo();
    MF->getContext().reportWarning(
        Inst.getLoc(), Twine("No known expansion for pseudo-instruction ") +
                           TII->getName(Inst.getOpcode()));
    EmitToStreamer(*OutStreamer, Inst);
  }
}

StringRef STM8AsmPrinter::getInstName(unsigned Opcode) const {
  assert(MF && "Current MachineFunction pointer is not set!");

  auto *TII = MF->getSubtarget().getInstrInfo();
  return TII->getName(Opcode);
}

} // namespace llvm

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSTM8AsmPrinter() {
  llvm::RegisterAsmPrinter<llvm::STM8AsmPrinter> X(llvm::getTheSTM8Target());
}
