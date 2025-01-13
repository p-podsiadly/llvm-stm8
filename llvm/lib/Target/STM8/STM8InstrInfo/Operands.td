include "STM8RegisterInfo.td"

//==============================================================================
// Operand Classes
//==============================================================================

// ex: "ADD A, $55"
def DirectMemOperand : AsmOperandClass {
    let Name = "DirectMem";
    let ParserMethod = "parseDirectMemOperand";
}

// ex: "ADD A, (X)"
class DirectIndexedOperand<STM8RegClass RC> : AsmOperandClass {
    let Name = "DirectIndexed" # RC.Name;
    let ParserMethod = "parseDirectIndexedOperand" # RC.Name;
}

// ex: "ADD A, ($55,X)"
class DirectOffsetIndexedOperand<STM8RegClass RC> : AsmOperandClass {
    let Name = "DirectOffsetIndexed" # RC.Name;
    let ParserMethod = "parseDirectOffsetIndexedOperand" # RC.Name;
}


// ex: "ADD A, [$55.w]"
def IndirectMemOperand : AsmOperandClass {
    let Name = "IndirectMem";
    let ParserMethod = "parseIndirectMemOperand";
}

// ex: "ADD A, ([$55.w], X)"
def IndirectIndexedOperand : AsmOperandClass {
    let Name = "IndirectIndexed";
    let ParserMethod = "parseIndirectIndexedOperand";
}

//==============================================================================
// Operands
//==============================================================================

// Custom immediate operands. Used instead of i8imm/i16imm in order to have
// correct printing of negative values (instead of printing of sign-extended
// 64 bit integer).
let OperandType = "OPERAND_IMMEDIATE" in {
    def Imm8 : Operand<i8> {
        let PrintMethod = "printImm8";
        let EncoderMethod = "encodeImm8";
    }

    def Imm16 : Operand<i16> {
        let PrintMethod = "printImm16";
        let EncoderMethod = "encodeImm16";
    }
}

class DirectMem<Operand AddrOp> : Operand<iPTR> {
    let ParserMatchClass = DirectMemOperand;
    let MIOperandInfo = (ops AddrOp);
    let PrintMethod = "printDirectMemOperand";
    let EncoderMethod = "encodeDirectMem";
}

def DirectMem8 : DirectMem<Imm8> {}
def DirectMem16 : DirectMem<Imm16> {}

class DirectIndexed<STM8RegClass RC> : Operand<iPTR> {
    let ParserMatchClass = DirectIndexedOperand<RC>;
    let MIOperandInfo = (ops RC);
    let PrintMethod = "printDirectIndexedOperand";
}

def DirectIdxX : DirectIndexed<XREG>;
def DirectIdxY : DirectIndexed<YREG>;

def IR16 : DirectIndexed<XY>;

class DirectOffsetIndexed<STM8RegClass RC, Operand OffsetOp> : Operand<iPTR> {
    let ParserMatchClass = DirectOffsetIndexedOperand<RC>;
    let MIOperandInfo = (ops RC:$rc, OffsetOp);
    let EncoderMethod = "encodeOperandDOI";
    let PrintMethod = "printOperandDOI";
}

def DirectOff8IdxX  : DirectOffsetIndexed<XREG, Imm8>;
def DirectOff16IdxX : DirectOffsetIndexed<XREG, Imm16>;
def DirectOff8IdxY  : DirectOffsetIndexed<YREG, Imm8>;
def DirectOff16IdxY : DirectOffsetIndexed<YREG, Imm16>;
def DirectOff8SP    : DirectOffsetIndexed<SPREG, Imm8>;
def DirectOff16SP   : DirectOffsetIndexed<SPREG, Imm16>;

def IR16D8  : DirectOffsetIndexed<XY, Imm8>;
def IR16D16 : DirectOffsetIndexed<XY, Imm16>;

class IndirectMem<Operand AddrOp> : Operand<iPTR> {
    let ParserMatchClass = IndirectMemOperand;
    let MIOperandInfo = (ops AddrOp);
    let EncoderMethod = "encodeIndirectMem";
    let PrintMethod = "printIndirectMemOperand";
}

def IndirectMem8 : IndirectMem<Imm8> {}
def IndirectMem16 : IndirectMem<Imm16> {}

def IndirectIndexed : Operand<iPTR> {
    let ParserMatchClass = IndirectIndexedOperand;
}

def PCRel8 : Operand<OtherVT> {
    let OperandType = "OPERAND_PCREL";
    let ParserMatchClass = DirectMemOperand;
    let PrintMethod = "printPCRelImm";
    let EncoderMethod = "encodePCRelImm";
}

//==============================================================================
// Complex patterns
//==============================================================================

def IR16Pat  : ComplexPattern<iPTR, 1, "SelectIR16", [], []>;
def XYDispCP : ComplexPattern<iPTR, 2, "SelectXYDisp", [add], [SDNPWantParent]>;
def SPDispCP : ComplexPattern<iPTR, 2, "SelectSPDisp", [add, frameindex], [SDNPWantParent]>;

//==============================================================================
// Immediate patterns
//==============================================================================

def Imm8Pat  : PatLeaf<(i8 imm)>;
def Imm16Pat : PatLeaf<(i16 imm)>;
