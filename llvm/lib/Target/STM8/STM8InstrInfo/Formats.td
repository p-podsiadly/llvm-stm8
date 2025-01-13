//==============================================================================
// Operand decoding for disassembler
//==============================================================================

class OpDecOpcode<DAGOperand op> {
    field string Name = !cond(
        !isa<STM8RegClass>(op):   "OpDec_" # !cast<STM8RegClass>(op).Name,
        !eq(op, Imm8):            "OpDec_Imm8",
        !eq(op, Imm16):           "OpDec_Imm16",
        !eq(op, DirectMem8):      "OpDec_Imm8",
        !eq(op, DirectMem16):     "OpDec_Imm16",
        !eq(op, DirectIdxX):      "OpDec_X",
        !eq(op, DirectIdxY):      "OpDec_Y",
        !eq(op, DirectOff8IdxX):  "OpDec_X, OpDec_Imm8",
        !eq(op, DirectOff16IdxX): "OpDec_X, OpDec_Imm16",
        !eq(op, DirectOff8IdxY):  "OpDec_Y, OpDec_Imm8",
        !eq(op, DirectOff16IdxY): "OpDec_Y, OpDec_Imm16",
        !eq(op, DirectOff8SP):    "OpDec_SP, OpDec_Imm8",
        !eq(op, IndirectMem8):    "OpDec_Imm8",
        !eq(op, IndirectMem16):   "OpDec_Imm16",
        !eq(op, PCRel8):          "OpDec_Imm8",
        true:                     "OpDec_Fail",
    );
}

class OpDecOpcodesList<dag operands> {
    field list<string> Val = !foreach(idx, !range(0, !size(operands)),
        OpDecOpcode<!getdagarg<DAGOperand>(operands, idx)>.Name);
}

class OpDecMethod<dag ins, dag outs> {
    field string Name = "decodeOperands<" #
        !interleave(!listconcat(OpDecOpcodesList<outs>.Val, OpDecOpcodesList<ins>.Val), ", ")
        # ">";
}

//==============================================================================
// Classes for STM8-specific instructions
//==============================================================================

class STM8Inst<string asmStr, dag ins, dag outs, list<dag> pattern = []> : Instruction {

    let Namespace = "STM8";
    let AsmString = asmStr;
    let Pattern = pattern;
    let DecoderNamespace = "STM8";

    let InOperandList = ins;
    let OutOperandList = outs;

    let DecoderMethod = OpDecMethod<InOperandList, OutOperandList>.Name;
}

/*
 * STM8-specific pseudo-instruction.
 * Such instructions are not part of the instruction set but simplify instruction selection.
 * They are expanded to real instructions after register allocation.
 */
class STM8Pseudo<string mnemonic, dag ins, dag outs, list<dag> pattern = []>
    : STM8Inst<"P" # mnemonic, ins, outs, pattern> {
  let isPseudo = 1;
}

/*
 * Generic instructions.
 * STM8 uses different opcodes depending on which register is used as an operand.
 * Since registers are not known during instruction selection, the STM8 backend uses
 * generic instructions which correspond to real instructions but work with any register
 * in a Gpr8/Gpr16 class.
 * Generic instructions have to be replaced with real instructions after register allocation.
 */
class STM8GenInst<string mnemonic, dag ins, dag outs, list<dag> pattern = []>
    : STM8Inst<"gen" # mnemonic, ins, outs, pattern> {
    let isPseudo = 1;
}
