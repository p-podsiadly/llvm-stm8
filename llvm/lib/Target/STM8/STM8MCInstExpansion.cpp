#include "STM8MCInstExpansion.h"
#include "MCTargetDesc/STM8MCTargetDesc.h"
#include "STM8ISelLowering.h"
#include "STM8RegisterInfo.h"
#include <llvm/MC/MCInst.h>

namespace llvm {

namespace {

template <unsigned OpNum, unsigned Reg, typename Action>
auto IfOpIsReg(const Action &action) {
  return [=](const MCInst &Inst, ExpandedMCInst &EI) -> bool {
    if (Inst.getNumOperands() <= OpNum) {
      return false;
    }

    MCOperand &Op = Inst.getOperand(OpNum);
    if (!Op.isReg() || (Op.getReg() != Reg)) {
      return false;
    }

    return action(Inst, EI);
  };
}

template <unsigned OpNum, typename XAction, typename YAction>
auto IfOpIsXOrY(const XAction &xAct, const YAction &yAct) {
  return [=](const MCInst &Inst, ExpandedMCInst &EI) -> bool {
    if (Inst.getNumOperands() <= OpNum) {
      return false;
    }

    if (MCOperand Op = Inst.getOperand(OpNum); Op.isReg()) {
      if (Op.getReg() == STM8::X) {
        return xAct(Inst, EI);
      } else if (Op.getReg() == STM8::Y) {
        return yAct(Inst, EI);
      }
    }

    return false;
  };
}

auto SetOpcode(unsigned NewOpcode) {
  return [=](const MCInst &Inst, ExpandedMCInst &EI) -> bool {
    EI.push_back(Inst);
    EI.back().setOpcode(NewOpcode);
    return true;
  };
};

template <unsigned OpNum> auto SetXYOpcode(unsigned XOpcode, unsigned YOpcode) {
  return [=](const MCInst &Inst, ExpandedMCInst &Expansion) {
    if (Inst.getNumOperands() <= OpNum) {
      return false;
    }

    unsigned NewOpcode = 0;
    MCOperand Op = Inst.getOperand(OpNum);
    if (Op.isReg()) {
      if (Op.getReg() == STM8::X) {
        NewOpcode = XOpcode;
      } else if (Op.getReg() == STM8::Y) {
        NewOpcode = YOpcode;
      }
    }

    if (NewOpcode) {
      Expansion.push_back(Inst);
      Expansion.back().setOpcode(NewOpcode);
      return true;
    }

    return false;
  };
}

template <unsigned OpNum> auto SetAOpcode(unsigned AOpcode) {
  return [=](const MCInst &Inst, ExpandedMCInst &Expansion) {
    const MCOperand &Op = Inst.getOperand(OpNum);
    assert(Op.isReg() && (Op.getReg() == STM8::A) &&
           "Expected to get instruction with A reg operand");

    Expansion.push_back(Inst);
    Expansion.back().setOpcode(AOpcode);
    return true;
  };
}

template <unsigned... OpIndices> auto MapToInstr(unsigned NewOpcode) {
  return [=](const MCInst &Inst, ExpandedMCInst &Expansion) -> bool {
    Expansion.emplace_back();
    Expansion.back().setOpcode(NewOpcode);
    for (unsigned OpNum : {OpIndices...}) {
      Expansion.back().addOperand(Inst.getOperand(OpNum));
    }

    return true;
  };
}

template <unsigned OpNum> auto SetRegOperand(unsigned Reg) {
  return [=](const MCInst &Inst, ExpandedMCInst &Expansion) -> bool {
    if (OpNum >= Inst.getNumOperands()) {
      return false;
    }

    Expansion.push_back(Inst);
    Expansion.back().getOperand(OpNum) = MCOperand::createReg(Reg);

    return true;
  };
}

bool DoAllImpl(ExpandedMCInst &Expansion) { return true; }

template <typename Act1, typename... Actions>
bool DoAllImpl(ExpandedMCInst &Expansion, const Act1 &act1,
               const Actions &...actions) {
  ExpandedMCInst NewExpansion;
  for (const auto &PrevInst : Expansion) {
    if (!act1(PrevInst, NewExpansion)) {
      return false;
    }
  }

  if (!DoAllImpl(NewExpansion, actions...)) {
    return false;
  }

  Expansion = std::move(NewExpansion);
  return true;
}

template <typename... Actions> auto DoAll(const Actions &...actions) {
  return [&](const MCInst &Inst, ExpandedMCInst &Expansion) -> bool {
    ExpandedMCInst Exp;
    Exp.push_back(Inst);

    if (!DoAllImpl(Exp, actions...)) {
      return false;
    }

    Expansion = std::move(Exp);
    return true;
  };
}

auto ExpandPseudoADCW(unsigned AddwOpcode, unsigned IncwOpcode,
                      unsigned IncwInstSize) {
  return [=](const MCInst &Inst, ExpandedMCInst &Expansion) -> bool {
    MCInst Addw;
    Addw.setOpcode(AddwOpcode);
    for (unsigned OpIdx = 0; OpIdx < Inst.getNumOperands(); ++OpIdx) {
      Addw.addOperand(Inst.getOperand(OpIdx));
    }

    MCInst Jrnc;
    Jrnc.setOpcode(STM8::JRNC);
    Jrnc.addOperand(MCOperand::createImm(IncwInstSize));

    MCInst Incw;
    Incw.setOpcode(IncwOpcode);
    Incw.addOperand(Inst.getOperand(0));

    Expansion.push_back(Addw);
    Expansion.push_back(Jrnc);
    Expansion.push_back(Incw);

    return true;
  };
}

auto ExpandLoadEffectiveAddr(unsigned LDWOpcode, unsigned ADDWOpcode) {
  return [=](const MCInst &Inst, ExpandedMCInst &Expansion) -> bool {
    if (Inst.getNumOperands() != 3) {
      return false;
    }

    MCInst LDW;
    LDW.setOpcode(LDWOpcode);
    LDW.addOperand(Inst.getOperand(0));
    LDW.addOperand(Inst.getOperand(1));

    MCInst ADDW;
    ADDW.setOpcode(ADDWOpcode);
    ADDW.addOperand(Inst.getOperand(0));
    ADDW.addOperand(Inst.getOperand(0));
    ADDW.addOperand(Inst.getOperand(2));

    Expansion.push_back(LDW);
    Expansion.push_back(ADDW);

    return true;
  };
}

template <typename... Actions>
ExpandedMCInst DoExpand(const MCInst &Inst, const Actions &...actions) {
  ExpandedMCInst EI;
  (actions(Inst, EI) || ...);
  return EI;
}

} // namespace

ExpandedMCInst ExpandSTM8Inst(const MCInst &Inst) {
  unsigned Opc = Inst.getOpcode();

  ExpandedMCInst Expansion;
  switch (Opc) {
  default:
    // Return empty vector to let the caller know that this instruction has not
    // been expanded.
    break;
  // LD
  case STM8::GenLDair:
    Expansion = DoExpand(Inst, SetXYOpcode<1>(STM8::LDaix, STM8::LDaiy));
    break;
  // LDW
  case STM8::GenLDWri16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::LDWxi16, STM8::LDWyi16));
    break;
  case STM8::GenLDWrm16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::LDWxm16, STM8::LDWym16));
    break;
  case STM8::GenLDWm16r:
    Expansion = DoExpand(Inst, SetXYOpcode<1>(STM8::LDWm16x, STM8::LDWm16y));
    break;
  case STM8::GenLDWrir:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::LDWxix, STM8::LDWyiy));
    break;
  case STM8::GenLDWrird8:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::LDWxixd8, STM8::LDWyiyd8));
    break;
  case STM8::GenLDWirr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::LDWixy, STM8::LDWiyx));
    break;
  case STM8::GenLDWird8r:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::LDWixd8y, STM8::LDWiyd8x));
    break;
  case STM8::GenLDWrispd8:
    Expansion =
        DoExpand(Inst, SetXYOpcode<0>(STM8::LDWxispd8, STM8::LDWyispd8));
    break;
  case STM8::GenLDWispd8r:
    Expansion =
        DoExpand(Inst, SetXYOpcode<2>(STM8::LDWispd8x, STM8::LDWispd8y));
    break;
  // ADCW
  case STM8::PseudoADCWri16:
    Expansion = DoExpand(
        Inst, IfOpIsXOrY<0>(ExpandPseudoADCW(STM8::ADDWxi16, STM8::INCWx, 1),
                            ExpandPseudoADCW(STM8::ADDWyi16, STM8::INCWy, 2)));
    break;
  case STM8::PseudoADCWrispd8:
    Expansion = DoExpand(
        Inst,
        IfOpIsXOrY<0>(ExpandPseudoADCW(STM8::ADDWxispd8, STM8::INCWx, 1),
                      ExpandPseudoADCW(STM8::ADDWyispd8, STM8::INCWy, 2)));
    break;
  // ADDW/SUBW
  case STM8::GenADDWri16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::ADDWxi16, STM8::ADDWyi16));
    break;
  case STM8::GenADDWrispd8:
    Expansion =
        DoExpand(Inst, SetXYOpcode<0>(STM8::ADDWxispd8, STM8::ADDWyispd8));
    break;
  case STM8::GenADDWrm16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::ADDWxm16, STM8::ADDWym16));
    break;
  case STM8::GenSUBWri16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SUBWxi16, STM8::SUBWyi16));
    break;
  case STM8::GenSUBWrispd8:
    Expansion =
        DoExpand(Inst, SetXYOpcode<0>(STM8::SUBWxispd8, STM8::SUBWyispd8));
    break;
  case STM8::GenSUBWrm16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SUBWxm16, STM8::SUBWym16));
    break;
  // CLR/CLRW
  case STM8::GenCLRir:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::CLRix, STM8::CLRiy));
    break;
  case STM8::GenCLRird8:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::CLRixd8, STM8::CLRiyd8));
    break;
  case STM8::GenCLRird16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::CLRixd16, STM8::CLRiyd16));
    break;
  case STM8::GenCLRWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::CLRWx, STM8::CLRWy));
    break;
  // CP, CPW
  case STM8::GenCPWri16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::CPWxi16, STM8::CPWyi16));
    break;
  case STM8::GenCPWrir:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::CPWxiy, STM8::CPWyix));
    break;
  // CPL, CLPW
  case STM8::GenCPLir:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::CPLix, STM8::CPLiy));
    break;
  case STM8::GenCPLird8:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::CPLixd8, STM8::CPLiyd8));
    break;
  case STM8::GenCPLird16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::CPLixd16, STM8::CPLiyd16));
    break;
  case STM8::GenCPLWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::CPLWx, STM8::CPLWy));
    break;
  // LDLO, LDHI
  case STM8::GenLDarl:
    Expansion = DoExpand(
        Inst, IfOpIsXOrY<1>(
                  DoAll(SetOpcode(STM8::LDaxl), SetRegOperand<1>(STM8::XL)),
                  DoAll(SetOpcode(STM8::LDayl), SetRegOperand<1>(STM8::YL))));
    break;
  case STM8::GenLDarh:
    Expansion = DoExpand(
        Inst, IfOpIsXOrY<1>(
                  DoAll(SetOpcode(STM8::LDaxh), SetRegOperand<1>(STM8::XH)),
                  DoAll(SetOpcode(STM8::LDayh), SetRegOperand<1>(STM8::YH))));
    break;
  case STM8::GenLDrla:
    Expansion = DoExpand(
        Inst,
        IfOpIsXOrY<0>(
            DoAll(MapToInstr<0, 2>(STM8::LDxla), SetRegOperand<0>(STM8::XL)),
            DoAll(MapToInstr<0, 2>(STM8::LDyla), SetRegOperand<0>(STM8::YL))));
    break;
  case STM8::GenLDrha:
    Expansion = DoExpand(
        Inst,
        IfOpIsXOrY<0>(
            DoAll(MapToInstr<0, 2>(STM8::LDxha), SetRegOperand<0>(STM8::XH)),
            DoAll(MapToInstr<0, 2>(STM8::LDyha), SetRegOperand<0>(STM8::YH))));
    break;
  // INC/DEC, INCW/DECW
  case STM8::GenINCWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::INCWx, STM8::INCWy));
    break;
  case STM8::GenDECWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::DECWx, STM8::DECWy));
    break;
  // NEG
  case STM8::GenNEGWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::NEGWx, STM8::NEGWy));
    break;
  // Shifts, rotates
  case STM8::GenSLLir:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SLLix, STM8::SLLiy));
    break;
  case STM8::GenSLLird8:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SLLixd8, STM8::SLLiyd8));
    break;
  case STM8::GenSLLird16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SLLixd16, STM8::SLLiyd16));
    break;
  case STM8::GenSRLir:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SRLix, STM8::SRLiy));
    break;
  case STM8::GenSRLird8:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SRLixd8, STM8::SRLiyd8));
    break;
  case STM8::GenSRLird16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SRLixd16, STM8::SRLiyd16));
    break;
  case STM8::GenSRAir:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SRAix, STM8::SRAiy));
    break;
  case STM8::GenSRAird8:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SRAixd8, STM8::SRAiyd8));
    break;
  case STM8::GenSRAird16:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SRAixd16, STM8::SRAiyd16));
    break;
  case STM8::GenSLLWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SLLWx, STM8::SLLWy));
    break;
  case STM8::GenSRLWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SRLWx, STM8::SRLWy));
    break;
  case STM8::GenSRAWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SRAWx, STM8::SRAWy));
    break;
  case STM8::GenRLCWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::RLCWx, STM8::RLCWy));
    break;
  case STM8::GenRRCWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::RRCWx, STM8::RRCWy));
    break;
  // MUL
  case STM8::GenMULra:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::MULxa, STM8::MULya));
    break;
  // DIV
  case STM8::GenDIVra:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::DIVxa, STM8::DIVya));
    break;
  // TNZ/TNZW
  case STM8::GenTNZr:
    Expansion = DoExpand(Inst, SetAOpcode<0>(STM8::TNZa));
    break;
  case STM8::GenTNZWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::TNZWx, STM8::TNZWy));
    break;
  // SWAPW
  case STM8::GenSWAPWr:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::SWAPWx, STM8::SWAPWy));
    break;
  // Jumps
  case STM8::GenJPir:
    Expansion = DoExpand(Inst, SetXYOpcode<0>(STM8::JPix, STM8::JPiy));
    break;
  // LoadEffectiveAddr
  case STM8::LoadEffectiveAddr:
    Expansion = DoExpand(
        Inst,
        IfOpIsXOrY<0>(ExpandLoadEffectiveAddr(STM8::LDWxsp, STM8::ADDWxi16),
                      ExpandLoadEffectiveAddr(STM8::LDWysp, STM8::ADDWyi16)));
    break;
  }

  return Expansion;
}

} // namespace llvm
