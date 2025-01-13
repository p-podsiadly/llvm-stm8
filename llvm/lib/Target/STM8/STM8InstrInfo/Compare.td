multiclass Compare_Fmt<string mnemonic, bits<4> baseOpCode, STM8RegClass LHS> {
    defvar isA = !eq(LHS, AREG);
    defvar isY = !eq(LHS, YREG);
    
    if isA then {
        // CPai8
        def i8 : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, Imm8:$rhs),(outs)> {
            bits<8> rhs;
            
            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 12} = 0xA;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = rhs;
        }
    } else {
        // CPWxi16, CPWyi16
        def i16 : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, Imm16:$rhs),(outs)> {
            bits<16> rhs;
            
            let Size = !add(3, isY);
            field bits<32> Inst;
            let Inst{31 - 24} = 0x90;
            let Inst{23 - 20} = 0xA;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = rhs;
        }
    }

    // CPam8, CPWxm8, CPWym8
    def m8 : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, DirectMem8:$rhs), (outs)> {
        bits<8> rhs;

        let Size = !add(2, isY);
        field bits<32> Inst;
        let Inst{23 - 16} = 0x90;
        let Inst{15 - 12} = 0xB;
        let Inst{11 - 8} = baseOpCode;
        let Inst{7 - 0} = rhs;
    }

    // CPam16, CPWxm16, CPWym16
    def m16 : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, DirectMem16:$rhs), (outs)> {
        bits<16> rhs;

        let Size = !add(3, isY);
        field bits<32> Inst;
        let Inst{31 - 24} = 0x90;
        let Inst{23 - 20} = 0xC;
        let Inst{19 - 16} = baseOpCode;
        let Inst{15 - 0} = rhs;
    }

    // CP/CPW with X on the right side
    if !or(isA, isY) then {
        // CPaix, CPWyix
        def ix : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, DirectIdxX:$rhs), (outs)> {
            let Size = 1;
            field bits<8> Inst;
            let Inst{7 - 4} = 0xF;
            let Inst{3 - 0} = baseOpCode;
        }

        // CPaixd16, CPWyixd16
        def ixd16 : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, DirectOff16IdxX:$rhs), (outs)> {
            bits<16> rhs;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 20} = 0xD;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = rhs;
        }
    }

    // CP/CPW with Y on the right side
    if !or(isA, !not(isY)) then {
        // CPaiy, CPWxiy
        def iy : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, DirectIdxY:$rhs), (outs)> {
            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 8} = 0x90;
            let Inst{7 - 4} = 0xF;
            let Inst{3 - 0} = baseOpCode;
        }

        // CPaiyd16, CPWxiyd16
        def iyd16 : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, DirectOff16IdxY:$rhs), (outs)> {
            bits<16> rhs;

            let Size = 4;
            field bits<32> Inst;
            let Inst{31 - 24} = 0x90;
            let Inst{23 - 20} = 0xD;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = rhs;
        }
    }

    // CPW Y,(disp,SP) is not part of the instruction set.
    if !not(isY) then {
        // CPaispd8, CPWxispd8
        def ispd8 : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, DirectOff8SP:$rhs), (outs)> {
            bits<8> rhs;

            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 12} = 0x1;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = rhs;
        }
    }

    // CPaim8, CPWxim8, CPWyim8
    def im8 : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, IndirectMem8:$rhs), (outs)> {
        bits<8> rhs;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 16} = !if(isY, 0x91,0x92);
        let Inst{15 - 12} = 0xC;
        let Inst{11 - 8} = baseOpCode;
        let Inst{7 - 0} = rhs;
    }

    // CPaim16, CPWxim16
    // Indirect Y-indexed addressing with long offset is not part of the instruction set.
    if !not(isY) then {
        def im16 : STM8Inst<mnemonic # "\t$lhs,$rhs", (ins LHS:$lhs, IndirectMem16:$rhs), (outs)> {
            bits<16> rhs;

            let Size = 4;
            field bits<32> Inst;
            let Inst{31 - 24} = 0x72;
            let Inst{23 - 20} = 0xC;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = rhs;
        }
    }
}

class PseudoCompare_Fmt<string mnemonic, STM8RegClass LhsRC, STM8RegClass RhsRC>
    : STM8Pseudo<mnemonic # "\t$lhs,$rhs", (ins LhsRC:$lhs, RhsRC:$rhs), (outs)>;

let Defs = [CCv, CCn, CCz, CCc] in {
    defm CPa  : Compare_Fmt<"cp  ", 0x1, AREG>;
    defm CPWx : Compare_Fmt<"cpw ", 0x3, XREG>;
    defm CPWy : Compare_Fmt<"cpw ", 0x3, YREG>;

    def PseudoCPab  : PseudoCompare_Fmt<"cp  ", AREG, BREG>;
    def PseudoCPWxz : PseudoCompare_Fmt<"cpw ", XREG, ZREG>;

    def GenCPWri16 : STM8GenInst<"cpw\t$lhs,$rhs", (ins XY:$lhs, Imm16:$rhs), (outs)>;

    def GenCPWrir : STM8GenInst<"cpw\t$lhs,$rhs", (ins XY:$lhs, IR16:$rhs), (outs)>;
}

//==============================================================================
// Test for Negative or Zero
//==============================================================================

class TNZInst<DAGOperand opCls, bits<16> opCode, int numSrcBytes>
    : STM8Inst<"tnz \t$src", (ins opCls:$src), (outs), []> {
    let Defs = [CCn, CCz];

    let Size = !add(numSrcBytes, 1, !ne(opCode{15 - 8}, 0x0));
    field bits<32> Inst = 0x0;
}

class TNZInst0<DAGOperand opCls, bits<16> opCode> : TNZInst<opCls, opCode, 0> {
    let Inst{15 - 0} = opCode;
}

class TNZInst8<DAGOperand opCls, bits<16> opCode> : TNZInst<opCls, opCode, 1> {
    bits<8> src;
    let Inst{23 - 8} = opCode;
    let Inst{7 - 0} = src;
}

class TNZInst16<DAGOperand opCls, bits<16> opCode> : TNZInst<opCls, opCode, 2> {
    bits<16> src;
    let Inst{31 - 16} = opCode;
    let Inst{15 - 0} = src;
}

def TNZa     : TNZInst0<AREG, 0x4d>;

let mayLoad = true in {
    def TNZm8    : TNZInst8<DirectMem8, 0x3d>;
    def TNZm16   : TNZInst16<DirectMem16, 0x725d>;
    def TNZix    : TNZInst0<DirectIdxX, 0x7d>;
    def TNZixd8  : TNZInst8<DirectOff8IdxX, 0x6d>;
    def TNZixd16 : TNZInst16<DirectOff16IdxX, 0x724d>;
    def TNZiy    : TNZInst0<DirectIdxY, 0x907d>;
    def TNZiyd8  : TNZInst8<DirectOff8IdxY, 0x906d>;
    def TNZiyd16 : TNZInst16<DirectOff16IdxY, 0x904d>;
    def TNZispd8 : TNZInst8<DirectOff8SP, 0x0d>;
    def TNZim8   : TNZInst8<IndirectMem8, 0x923d>;
    def TNZim16  : TNZInst16<IndirectMem16, 0x723d>;
}

//==============================================================================
// Word Test for Negative or Zero
//==============================================================================

// TNZWx, TNZWy
foreach RC = [XREG, YREG] in {
    def "TNZW" # RC.RS : STM8Inst<"tnzw\t$src", (ins RC:$src), (outs), []> {
        let Defs = [CCn, CCz];

        let Size = !add(1, !eq(RC, YREG));
        field bits<16> Inst = 0x905d;
    }    
}

//==============================================================================
// Generic and pseudo-instructions
//==============================================================================

let Defs = [CCn, CCz] in {
    def PseudoTNZb  : STM8Pseudo<"tnz\t$src", (ins BREG:$src), (outs)>;

    def GenTNZr  : STM8GenInst<"tnz\t$src", (ins AREG:$src), (outs)>;
    def GenTNZWr : STM8GenInst<"tnzw\t$src", (ins XY:$src), (outs)>;
}
