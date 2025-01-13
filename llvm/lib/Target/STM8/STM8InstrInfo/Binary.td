//==============================================================================
// Pattern fragments
//==============================================================================

def STM8AddPat : PatFrags<
    (ops node:$lhs, node:$rhs), [
        (STM8Add node:$lhs, node:$rhs),
        (add node:$lhs, node:$rhs)
]>;

//==============================================================================
// 8-bit binary operations
//==============================================================================

multiclass BinaryDstA_Fmt<bits<4> baseOpCode, string mnemonic, SDPatternOperator opNode> {
    defvar hasPat = !ne(opNode, undef);

    let Constraints = "$dst = $lhs" in {
        def i8 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins AREG:$lhs, Imm8:$rhs), (outs AREG:$dst),
            !if(hasPat, [(set i8:$dst, (opNode i8:$lhs, Imm8Pat:$rhs))], [])> {
            bits<8> rhs;

            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 12} = 0xA;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = rhs;
        }

        def m8 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins AREG:$lhs, DirectMem8:$rhs), (outs AREG:$dst),
            !if(hasPat, [(set i8:$dst, (opNode i8:$lhs, (load i8:$rhs)))], [])> {
            bits<8> rhs;

            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 12} = 0xB;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = rhs;
        }

        def m16 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins AREG:$lhs, DirectMem16:$rhs), (outs AREG:$dst),
            !if(hasPat, [(set i8:$dst, (opNode i8:$lhs, (load i16:$rhs)))], [])> {
            bits<16> rhs;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 20} = 0xC;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = rhs;
        }

        foreach isY = [0, 1] in {
            defvar RC = !if(isY, YREG, XREG);

            def "i" # RC.RS : STM8Inst<mnemonic # "\t$dst,$rhs",
                (ins AREG: $lhs, DirectIndexed<RC>:$rhs), (outs AREG: $dst), []> {
                let Size = !add(1, isY);
                field bits<16> Inst;
                let Inst{15 - 4} = 0x90F;
                let Inst{3 - 0} = baseOpCode;
            }

            def "i" # RC.RS # "d8" : STM8Inst<mnemonic # "\t$dst,$rhs",
                (ins AREG: $lhs, DirectOffsetIndexed<RC, Imm8>:$rhs), (outs AREG: $dst), []> {
                bits<8> rhs;
                let Size = !add(2, isY);
                field bits<24> Inst;
                let Inst{23 - 12} = 0x90E;
                let Inst{11 - 8} = baseOpCode;
                let Inst{7 - 0} = rhs;
            }

            def "i" # RC.RS # "d16" : STM8Inst<mnemonic # "\t$dst,$rhs",
                (ins AREG: $lhs, DirectOffsetIndexed<RC, Imm16>:$rhs), (outs AREG: $dst), []> {
                bits<16> rhs;
                let Size = !add(3, isY);
                field bits<32> Inst;
                let Inst{31 - 20} = 0x90D;
                let Inst{19 - 16} = baseOpCode;
                let Inst{15 - 0} = rhs;
            }
        }

        def ispd8 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins AREG:$lhs, DirectOff8SP:$rhs), (outs AREG:$dst),
            !if(hasPat, [(set i8:$dst, (opNode i8:$lhs, (load SPDispCP:$rhs)))], [])> {
            bits<8> rhs;

            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 12} = 0x1;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = rhs;
        }

        def im8 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins AREG:$lhs, IndirectMem8:$rhs), (outs AREG:$dst),
            !if(hasPat, [(set i8:$dst, (opNode i8:$lhs, (load Imm8Pat:$rhs)))], [])> {
            bits<8> rhs;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 12} = 0x92C;
            let Inst{11 - 8} = baseOpCode;
            let Inst{7 - 0} = rhs;
        }

        def im16 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins AREG:$lhs, IndirectMem16:$rhs), (outs AREG:$dst),
            !if(hasPat, [(set i8:$dst, (opNode i8:$lhs, (load Imm16Pat:$rhs)))], [])> {
            bits<16> rhs;

            let Size = 4;
            field bits<32> Inst;
            let Inst{31 - 20} = 0x72C;
            let Inst{19 - 16} = baseOpCode;
            let Inst{15 - 0} = rhs;
        }
    }
}

// Instruction accepting B registers as the right hand side.
multiclass PseudoBinaryAB_Fmt<string mnemonic, SDPatternOperator opNode> {
    defvar hasPat = !ne(opNode, undef);

    let Constraints = "$dst = $lhs" in {
        def ab : STM8Pseudo<mnemonic # "\t$dst,$rhs",
            (ins AREG:$lhs, BREG:$rhs), (outs AREG:$dst),
            !if(hasPat, [(set i8:$dst, (opNode i8:$lhs, i8:$rhs))], [])>;
    }
}

let Uses = [CCc], Defs = [CCv, CCh, CCn, CCz, CCc] in {
    defm ADCa      : BinaryDstA_Fmt<0x9, "adc ", undef>;
    defm PseudoADC : PseudoBinaryAB_Fmt<"adc ", undef>;
}

let Uses = [CCc], Defs = [CCv, CCn, CCz, CCc] in {
    defm SBCa      : BinaryDstA_Fmt<0x2, "sbc ", undef>;
    defm PseudoSBC : PseudoBinaryAB_Fmt<"sbc ", undef>;
}

let Defs = [CCv, CCh, CCn, CCz, CCc] in {
    defm ADDa      : BinaryDstA_Fmt<0xB, "add ", STM8AddPat>;
    defm PseudoADD : PseudoBinaryAB_Fmt<"add ", STM8AddPat>;
}

let Defs = [CCv, CCn, CCz, CCc] in {
    defm SUBa      : BinaryDstA_Fmt<0x0, "sub ", sub>;
    defm PseudoSUB : PseudoBinaryAB_Fmt<"sub ", sub>;

    def SUBspi8 : STM8Inst<"sub \t$dst,$rhs",
        (ins SPREG:$lhs, Imm8:$rhs), (outs SPREG:$dst),
        [(set i16:$dst, (sub i16:$lhs, (zext Imm8Pat:$rhs)))]> {
        bits<8> rhs;

        let Constraints = "$dst = $lhs";
        let Size = 2;
        field bits<16> Inst;
        let Inst{15 - 8} = 0x52;
        let Inst{7 - 0} = rhs;
    }
}

let Defs = [CCn, CCz] in {
    defm ANDa      : BinaryDstA_Fmt<0x4, "and ", and>;
    defm PseudoAND : PseudoBinaryAB_Fmt<"and ", and>;

    defm ORa      : BinaryDstA_Fmt<0xA, "or  ", or>;
    defm PseudoOR : PseudoBinaryAB_Fmt<"or ", or>;

    defm XORa      : BinaryDstA_Fmt<0x8, "xor ", xor>;
    defm PseudoXOR : PseudoBinaryAB_Fmt<"xor", xor>;
}

//==============================================================================
// Word instructions
//==============================================================================

class AddSubW<string mnemonic> {
    field bit isAdd = !eq(mnemonic, "addw");
    field bit isSub = !eq(mnemonic, "subw");

    field SDPatternOperator Op = !cond(isAdd: STM8AddPat, isSub: sub);
}

multiclass AddSubW_Fmt<string mnemonic> {
    defvar isAdd = AddSubW<mnemonic>.isAdd;
    defvar opNode = AddSubW<mnemonic>.Op;

    defvar baseOpCode0 = !cond(isAdd: 0xC, true: 0xD);
    defvar baseOpCode1 = !cond(isAdd: 0xB, true: 0x0);
    defvar baseOpCode2 = !cond(isAdd: 0x9, true: 0x2);

    let Constraints = "$dst = $lhs" in {

        def xi16 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins XREG:$lhs, Imm16:$rhs), (outs XREG:$dst), []> {
            bits<16> rhs;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 20} = 0x1;
            let Inst{19 - 16} = baseOpCode0;
            let Inst{15 - 0} = rhs;
        }

        def xm16 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins XREG:$lhs, DirectMem16:$rhs), (outs XREG:$dst), []> {
            bits<16> rhs;

            let Size = 4;
            field bits<32> Inst;
            let Inst{31 - 20} = 0x72B;
            let Inst{19 - 16} = baseOpCode1;
            let Inst{15 - 0} = rhs;
        }

        def xispd8 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins XREG:$lhs, DirectOff8SP:$rhs), (outs XREG:$dst), []> {
            bits<8> rhs;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 12} = 0x72F;
            let Inst{11 - 8} = baseOpCode1;
            let Inst{7 - 0} = rhs;
        }

        def yi16 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins YREG:$lhs, Imm16:$rhs), (outs YREG:$dst), []> {
            bits<16> rhs;

            let Size = 4;
            field bits<32> Inst;
            let Inst{31 - 20} = 0x72A;
            let Inst{19 - 16} = baseOpCode2;
            let Inst{15 - 0} = rhs;
        }

        def ym16 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins YREG:$lhs, DirectMem16:$rhs), (outs YREG:$dst), []> {
            bits<16> rhs;

            let Size = 4;
            field bits<32> Inst;
            let Inst{31 - 20} = 0x72B;
            let Inst{19 - 16} = baseOpCode2;
            let Inst{15 - 0} = rhs;
        }

        def yispd8 : STM8Inst<mnemonic # "\t$dst,$rhs",
            (ins YREG:$lhs, DirectOff8SP:$rhs), (outs YREG:$dst), []> {
            bits<8> rhs;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 12} = 0x72F;
            let Inst{11 - 8} = baseOpCode2;
            let Inst{7 - 0} = rhs;
        }
    }
}

multiclass GenAddSubW_Fmt<string mnemonic> {
    defvar opNode = AddSubW<mnemonic>.Op;

    let Constraints = "$dst = $lhs" in {
        def rz : STM8GenInst<mnemonic # "\t$dst,$rhs",
            (ins XY:$lhs, ZREG:$rhs), (outs XY:$dst),
            [(set i16:$dst, (opNode i16:$lhs, i16:$rhs))]>;

        def ri16 : STM8GenInst<mnemonic # "\t$dst,$rhs",
            (ins XY:$lhs, Imm16:$rhs), (outs XY:$dst),
            [(set i16:$dst, (opNode i16:$lhs, Imm16Pat:$rhs))]>;

        def rm16 : STM8GenInst<mnemonic # "\t$dst,$rhs",
            (ins XY:$lhs, DirectMem16:$rhs), (outs XY:$dst),
            [(set i16:$dst, (opNode i16:$lhs, (load Imm16Pat:$rhs)))]>;

        def rispd8 : STM8GenInst<mnemonic # "\t$dst,$rhs",
            (ins XY:$lhs, DirectOff8SP:$rhs), (outs XY:$dst),
            [(set i16:$dst, (opNode i16:$lhs, (load SPDispCP:$rhs)))]>;
    }
}

let Defs = [CCv, CCh, CCn, CCz, CCc] in {
    defm ADDW    : AddSubW_Fmt<"addw">;
    defm GenADDW : GenAddSubW_Fmt<"addw">;
}

def ADDWspi8 : STM8Inst<"addw\t$rd,$imm",
    (ins SPREG:$rr, Imm8:$imm), (outs SPREG:$rd),
    [(set i16:$rd, (add i16:$rr, (zext Imm8Pat:$imm)))]> {

    bits<8> imm;

    let Size = 2;
    field bits<16> Inst;
    let Inst{15 - 8} = 0x5B;
    let Inst{7 - 0} = imm;
}

let Defs = [CCv, CCh, CCn, CCz, CCc] in {
    defm SUBW    : AddSubW_Fmt<"subw">;
    defm GenSUBW : GenAddSubW_Fmt<"subw">;
}

// Pseudo-instructions for 16-bit addition with incoming carry

multiclass PseudoBinaryW<string mnemonic, SDPatternOperator opNode> {
    let Constraints = "$dst = $lhs" in {
        def rz : STM8Pseudo<mnemonic # "\t$lhs,$rhs",
            (ins XY:$lhs, ZREG:$rhs), (outs XY:$dst),
            [(set i16:$dst, (opNode i16:$lhs, i16:$rhs))]>;

        def ri16 : STM8Pseudo<mnemonic # "\t$lhs,$rhs",
            (ins XY:$lhs, Imm16:$rhs), (outs XY:$dst),
            [(set i16:$dst, (opNode i16:$lhs, Imm16Pat:$rhs))]>;

        def rispd8 : STM8Pseudo<mnemonic # "\t$lhs,$rhs",
            (ins XY:$lhs, DirectOff8SP:$rhs), (outs XY:$dst),
            [(set i16:$dst, (opNode i16:$lhs, (load SPDispCP:$rhs)))]>;
    }
}

let Uses = [CCc], Defs = [CCv, CCh, CCn, CCz, CCc] in {
    defm PseudoADCW : PseudoBinaryW<"adcw",
        PatFrags<(ops node:$lhs, node:$rhs),
            [(STM8AddCarry node:$lhs, node:$rhs, CCc)]>>;
}

//==============================================================================
// Variable shifts
//==============================================================================

let Constraints = "$dst = $lhs", usesCustomInserter = true,
    Defs = [CCn, CCz, CCc] in {

    // 8-bit instructions

    // Note: PseudoVARSxx instructions are expanded at the end of instruction
    // selection, before register allocation. For this reason it is safe to use
    // AREG for both inputs.

    def PseudoVarSLL : STM8Pseudo<"varsll\t$dst,$rhs",
        (ins AREG:$lhs, AREG:$rhs), (outs AREG:$dst),
        [(set i8:$dst, (STM8VarSHL i8:$lhs, i8:$rhs))]>;

    def PseudoVarSRL : STM8Pseudo<"varsrl\t$dst,$rhs",
        (ins AREG:$lhs, AREG:$rhs), (outs AREG:$dst),
        [(set i8:$dst, (STM8VarSHR i8:$lhs, i8:$rhs))]>;

    def PseudoVarSRA : STM8Pseudo<"varsra\t$dst,$rhs",
        (ins AREG:$lhs, AREG:$rhs), (outs AREG:$dst),
        [(set i8:$dst, (STM8VarASHR i8:$lhs, i8:$rhs))]>;

    // 16-bit instructions

    def PseudoVarSLLW : STM8Pseudo<"varsllw\t$dst,$rhs",
        (ins XY:$lhs, AREG:$rhs), (outs XY:$dst),
        [(set i16:$dst, (STM8VarSHL i16:$lhs, i8:$rhs))]>;

    def PseudoVarSRLW : STM8Pseudo<"varsrlw\t$dst,$rhs",
        (ins XY:$lhs, AREG:$rhs), (outs XY:$dst),
        [(set i16:$dst, (STM8VarSHR i16:$lhs, i8:$rhs))]>;

    def PseudoVarSRAW : STM8Pseudo<"varsraw\t$dst,$rhs",
        (ins XY:$lhs, AREG:$rhs), (outs XY:$dst),
        [(set i16:$dst, (STM8VarASHR i16:$lhs, i8:$rhs))]>;
}

let usesCustomInserter = true, Defs = [CCn, CCz, CCc],
    Constraints = "$res_lo = $lhs_lo, $res_hi = $lhs_hi" in {

    def PseudoVarSLLWParts : STM8Pseudo<"varsllw_parts",
        (ins XY:$lhs_lo, XY:$lhs_hi, AREG:$amt),
        (outs XY:$res_lo, XY:$res_hi)>;

    def PseudoVarSRLWParts : STM8Pseudo<"varsllw_parts",
        (ins XY:$lhs_lo, XY:$lhs_hi, AREG:$amt),
        (outs XY:$res_lo, XY:$res_hi)>;
}

def : Pat<(STM8SHL_PARTS i16:$lhs_lo, i16:$lhs_hi, i8:$amt),
    (PseudoVarSLLWParts XY:$lhs_lo, XY:$lhs_hi, AREG:$amt)>;

def : Pat<(STM8SHL_PARTS i16:$lhs_lo, i16:$lhs_hi, i8:$amt),
    (PseudoVarSLLWParts XY:$lhs_lo, XY:$lhs_hi, AREG:$amt)>;

def : Pat<(STM8SRL_PARTS i16:$lhs_lo, i16:$lhs_hi, i8:$amt),
    (PseudoVarSRLWParts XY:$lhs_lo, XY:$lhs_hi, AREG:$amt)>;
