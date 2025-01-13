
//==============================================================================
// Loads into registers
//==============================================================================

let Defs = [CCn, CCz] in {
    def GenLDWri16 : STM8GenInst<"ldw\t$dst,$src",
        (ins Imm16:$src), (outs XY:$dst),
        [(set i16:$dst, Imm16Pat:$src)]> {

        let isReMaterializable = true;
        let isAsCheapAsAMove = true;
        let isMoveImm = true;
    }

    def GenLDWrm16 : STM8GenInst<"ldw\t$dst,$src",
        (ins Imm16:$src), (outs XY:$dst), [(set i16:$dst, (load Imm16Pat:$src))]> {
        let mayLoad = true;
    }

    def GenLDWm16r : STM8GenInst<"ldw\t$dst,$src",
        (ins Imm16:$dst, XY:$src), (outs) , [(store i16:$src, Imm16Pat:$dst)]> {
        let mayLoad = true;
        let mayStore = true;
    }

    def GenLDWrir : STM8GenInst<"ldw\t$dst,$src",
        (ins IR16:$src), (outs XY:$dst), [(set i16:$dst, (load IR16Pat:$src))]> {
        let mayLoad = true;
        let Constraints = "$dst = $src";
    }

    def GenLDWrird8 : STM8GenInst<"ldw\t$dst,$src",
        (ins IR16D8:$src), (outs XY:$dst), [(set i16:$dst, (load XYDispCP:$src))]> {
        let mayLoad = true; 
        let Constraints = "$dst = $src.rc";
    }

    def GenLDWirr : STM8GenInst<"ldw\t$dst,$src",
        (ins IR16:$dst, XY:$src), (outs), [(store i16:$src, IR16Pat:$dst)]> {
        let mayStore = true;
    }

    def GenLDWird8r : STM8GenInst<"ldw\t$dst,$src",
        (ins IR16D8:$dst, XY:$src), (outs), [(store i16:$src, XYDispCP:$dst)]> {
        let mayStore = true;
    }

    def GenLDWrispd8 : STM8GenInst<"ldw\t$dst,$src",
        (ins IR16D8:$src), (outs XY:$dst), [(set i16:$dst, (load SPDispCP:$src))]> {
        let mayLoad = true;
    }

    def GenLDWispd8r : STM8GenInst<"ldw\t$dst,$src",
        (ins DirectOff8SP:$dst, XY:$src), (outs), [(store i16:$src, SPDispCP:$dst)]> {
        let mayStore = true;
    }
}

multiclass LDW_Fmt<STM8RegClass RC> {

    defvar isY = !eq(RC, YREG);

    // Register string indicating type of register operands of an instruction.
    // Used for instruction names.
    defvar rs = RC.RS;

    defvar DirectIdxRC = !if(!eq(RC, XREG), DirectIdxX, DirectIdxY);
    defvar DirectOff8IdxRC = !if(!eq(RC, XREG), DirectOff8IdxX, DirectOff8IdxY);
    defvar DirectOff16IdxRC = !if(!eq(RC, XREG), DirectOff16IdxX, DirectOff16IdxY);

    ///////////////////
    // Loads into RC //
    ///////////////////

    // LDWxi16, LDWyi16
    def rs # "i16" : STM8Inst<"ldw \t$dst,$src", (ins Imm16:$src), (outs RC:$dst), []> {
        bits<16> src;

        let Size = !add(3, isY);
        field bits<32> Inst;
        let Inst{31 - 16} = 0x90AE;
        let Inst{15 - 0} = src;

        let isReMaterializable = true;
        let isAsCheapAsAMove = true;
        let isMoveImm = true;
    }

    let mayLoad = true in {
        // LDWxm8, LDWxm16
        def rs # "m8" : STM8Inst<"ldw \t$dst,$src", (ins DirectMem8:$src), (outs RC:$dst), [(set i16:$dst, (load Imm8Pat:$src))]> {
            bits<8> src;

            let Size = !add(2, isY);
            field bits<24> Inst;
            let Inst{23 - 8} = 0x90BE;
            let Inst{7 - 0} = src;
        }

        // LDWxm16, LDWym16
        def rs # "m16" : STM8Inst<"ldw \t$dst,$src", (ins DirectMem16:$src), (outs RC:$dst), [(set i16:$dst, (load Imm16Pat:$src))]> {
            bits<16> src;

            let Size = !add(3, isY);
            field bits<32> Inst;
            let Inst{31 - 24} = 0x90;
            let Inst{23 - 16} = 0xCE;
            let Inst{15 - 0} = src;
        }

        // LDWxix, LDWyiy
        def rs # "i" #rs : STM8Inst<"ldw \t$dst,$src", (ins DirectIdxRC:$src), (outs RC:$dst)> {
            let Constraints = "$dst = $src";

            let Size = !add(1, isY);
            field bits<16> Inst = 0x90FE;
        }

        // LDWxixd8, LDWyiyd8
        def rs # "i" # rs # "d8" : STM8Inst<"ldw \t$dst,$src", (ins DirectOff8IdxRC:$src), (outs RC:$dst)> {
            let Constraints = "$dst = $src.rc";

            bits<8> src;

            let Size = !add(2, isY);
            field bits<24> Inst;
            let Inst{23 - 8} = 0x90EE;
            let Inst{7 - 0} = src;
        }

        // LDWxixd16, LDWyiyd16
        def rs # "i" # rs # "d16" : STM8Inst<"ldw \t$dst,$src", (ins DirectOff16IdxRC:$src), (outs RC:$dst), [(set i16:$dst, (load XYDispCP:$src))]> {
            let Constraints = "$dst = $src.rc";

            bits<16> src;

            let Size = !add(3, isY);
            field bits<32> Inst;
            let Inst{31 - 16} = 0x90DE;
            let Inst{15 - 0} = src;
        }

        // LDWxispd8, LDWyispd8
        def rs # "ispd8" : STM8Inst<"ldw \t$dst,$src", (ins DirectOff8SP:$src), (outs RC:$dst), [(set i16:$dst, (load SPDispCP:$src))]> {
            bits<8> src;

            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 8} = !if(isY, 0x16, 0x1E);
            let Inst{7 - 0} = src;

            let canFoldAsLoad = true;
        }

        // LDWxim8, LDWyim8
        def rs # "im8" : STM8Inst<"ldw \t$dst,$src", (ins IndirectMem8:$src), (outs RC:$dst), [(set i16:$dst, (load Imm8Pat:$src))]> {
            bits<8> src;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 8} = !if(isY, 0x91CE, 0x92CE);
            let Inst{7 - 0} = src;
        }

        if !not(isY) then {
            def xim16 : STM8Inst<"ldw \t$dst,$src", (ins IndirectMem16:$src), (outs RC:$dst), [(set i16:$dst, (load Imm16Pat:$src))]> {
                bits<16> src;

                let Size = 4;
                field bits<32> Inst;
                let Inst{31 - 16} = 0x72CE;
                let Inst{15 - 0} = src;
            }
        }
    }

    ////////////////////
    // Stores from RC //
    ////////////////////

    defvar RC2 = !if(isY, XREG, YREG);
    defvar rs2 = !if(isY, "x", "y");

    defvar DirectIdxRC2 = !if(!eq(RC2, XREG), DirectIdxX, DirectIdxY);
    defvar DirectOff8IdxRC2 = !if(!eq(RC2, XREG), DirectOff8IdxX, DirectOff8IdxY);
    defvar DirectOff16IdxRC2 = !if(!eq(RC2, XREG), DirectOff16IdxX, DirectOff16IdxY);

    let mayStore = true in {

        // LDWm8x, LDWm8y
        def "m8" # rs : STM8Inst<"ldw \t$dst,$src", (ins DirectMem8:$dst, RC:$src), (outs)> {
            bits<8> dst;

            let Size = !add(2, isY);
            field bits<24> Inst;
            let Inst{23 - 8} = 0x90BF;
            let Inst{7 - 0} = dst;
        }

        // LDWm16x, LDWm16y
        def "m16" # rs : STM8Inst<"ldw \t$dst,$src", (ins DirectMem16:$dst, RC:$src), (outs)> {
            bits<16> dst;

            let Size = !add(3, isY);
            field bits<32> Inst;
            let Inst{31 - 16} = 0x90CF;
            let Inst{15 - 0} = dst;
        }

        // LDWiyx, LDWixy
        def "i" # rs2 # rs : STM8Inst<"ldw \t$dst,$src", (ins DirectIdxRC2:$dst, RC:$src), (outs)> {
            let Size = !if(isY, 1, 2);
            field bits<16> Inst = 0x90FF;
        }

        // LDWiyd8x, LDWixd8y
        def "i" # rs2 # "d8" # rs : STM8Inst<"ldw \t$dst,$src", (ins DirectOff8IdxRC2:$dst, RC:$src), (outs)> {
            bits<8> dst;

            let Size = !if(isY, 2, 3);
            field bits<24> Inst;
            let Inst{23 - 8} = 0x90EF;
            let Inst{7 - 0} = dst;
        }

        // LDWiyd16x, LDWixd16y
        def "i" # rs2 # "d16" # rs : STM8Inst<"ldw \t$dst,$src", (ins DirectOff16IdxRC2:$dst, RC:$src), (outs)> {
            bits<16> dst;

            let Size = !if(isY, 3, 4);
            field bits<32> Inst;
            let Inst{31 - 16} = 0x90DF;
            let Inst{15 - 0} = dst;
        }

        // LDWispd8x, LDWispd8y
        def "ispd8" # rs : STM8Inst<"ldw \t$dst,$src", (ins DirectOff8SP:$dst, RC:$src), (outs)> {
            bits<8> dst;

            let Size = 2;
            field bits<16> Inst;
            let Inst{15 - 8} = !if(isY, 0x17, 0x1F);
            let Inst{7 - 0} = dst;
        }

        // LDWim8x, LDWim8y
        def "im8" # rs : STM8Inst<"ldw \t$dst,$src", (ins IndirectMem8:$dst, RC:$src), (outs)> {
            bits<8> dst;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 8} = !if(isY, 0x91CF, 0x92CF);
            let Inst{7 - 0} = dst;
        }

        if !not(isY) then {
            def im16x : STM8Inst<"ldw \t$dst,$src", (ins IndirectMem16:$dst, RC:$src), (outs)> {
                bits<16> dst;

                let Size = 4;
                field bits<32> Inst;
                let Inst{31 - 16} = 0x72CF;
                let Inst{15 - 0} = dst;
            }
        }
    }
}

let Defs = [CCn, CCz] in {
    defm LDW : LDW_Fmt<XREG>;
    defm LDW : LDW_Fmt<YREG>;
}

def PseudoLDWzispd8 : STM8Pseudo<"ldw \t$dst,$src", (ins DirectOff8SP:$src), (outs ZREG:$dst)>;
def PseudoLDWispd8z : STM8Pseudo<"ldw \t$dst,$src", (ins ZREG:$src), (outs DirectOff8SP:$dst)>;

//////////////////////////////
// Copies between registers //
//////////////////////////////

class LDWrr<RegisterClass SrcRC, RegisterClass DstRC, bits<8> opCode, bits<8> prefix = 0x0>
    : STM8Inst<"ldw \t$rd,$rs", (ins SrcRC:$rs), (outs DstRC:$rd), []> {
    let Size = !if(!eq(prefix, 0), 1, 2);
    field bits<16> Inst;
    let Inst{15 - 8} = prefix;
    let Inst{7 - 0} = opCode;
}

let Defs = [CCn, CCz] in {   
    def GenLDWrsp : STM8GenInst<"ldw\t$dst,$src",
        (ins SPREG:$src), (outs XY:$dst), []>;
    
    def GenLDWspr : STM8GenInst<"ldw\t$dst,$src",
        (ins XY:$src), (outs SPREG:$dst), []>;

    def LDWyx  : LDWrr<XREG,  YREG,  0x93, 0x90>;
    def LDWxy  : LDWrr<YREG,  XREG,  0x93>;
    def LDWxsp : LDWrr<SPREG, XREG,  0x96>;
    def LDWspx : LDWrr<XREG,  SPREG, 0x94>;
    def LDWysp : LDWrr<SPREG, YREG,  0x96, 0x90>;
    def LDWspy : LDWrr<YREG,  SPREG, 0x94, 0x90>;
}

//////////////////
// Loads into A //
//////////////////

def LDai8 : STM8Inst<"ld  \t$rd,$src", (ins Imm8:$src), (outs AREG:$rd), [(set i8:$rd, Imm8Pat:$src)]> {
    bits<8> src;

    let Size = 2;
    field bits<16> Inst;
    let Inst{15 - 8} = 0xA6;
    let Inst{7 - 0} = src{7 - 0};

    let isReMaterializable = true;
    let isAsCheapAsAMove = true;
    let isMoveImm = true;
    let Defs = [CCn, CCz];
}

let mayLoad = true, Defs = [CCn, CCz] in {
    def LDam8 : STM8Inst<"ld  \t$dst,$src", (ins DirectMem8:$src), (outs AREG:$dst), [(set i8:$dst, (load Imm8Pat:$src))]> {
        bits<8> src;

        let Size = 2;
        field bits<16> Inst;
        let Inst{15 - 8} = 0xB6;
        let Inst{7 - 0} = src;
    }

    def LDam16 : STM8Inst<"ld  \t$dst,$src", (ins DirectMem16:$src), (outs AREG:$dst), [(set i8:$dst, (load Imm16Pat:$src))]> {
        bits<16> src;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 16} = 0xC6;
        let Inst{15 - 0} = src;
    }

    // LDaix, LDaiy
    foreach RC = [XREG, YREG] in {
        defvar SrcOp = !if(!eq(RC, XREG), DirectIdxX, DirectIdxY);
        def LDai # RC.RS : STM8Inst<"ld  \t$dst,$src", (ins SrcOp:$src), (outs AREG:$dst)> {
            let Size = !add(1, !eq(RC, YREG));
            field bits<16> Inst = 0x90F6;
        }
    }

    def GenLDair : STM8GenInst<"ld\t$dst,$src",
        (ins IR16:$src), (outs AREG:$dst), [(set i8:$dst, (load IR16Pat:$src))]>;

    def LDaixd8 : STM8Inst<"ld  \t$dst,$src", (ins DirectOff8IdxX:$src), (outs AREG:$dst), []> {
        bits<8> src;

        let Size = 2;
        field bits<16> Inst;
        let Inst{15 - 8} = 0xE6;
        let Inst{7 - 0} = src;
    }

    def LDaixd16 : STM8Inst<"ld  \t$dst,$src", (ins DirectOff16IdxX:$src), (outs AREG:$dst), [(set i8:$dst, (load XYDispCP:$src))]> {
        bits<16> src;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 16} = 0xD6;
        let Inst{15 - 0} = src;
    }

    def LDaiyd8 : STM8Inst<"ld  \t$dst,$src", (ins DirectOff8IdxY:$src), (outs AREG:$dst), []> {
        bits<8> src;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 16} = 0x90;
        let Inst{15 - 8} = 0xE6;
        let Inst{7 - 0} = src;
    }

    def LDaiyd16 : STM8Inst<"ld  \t$dst,$src", (ins DirectOff16IdxY:$src), (outs AREG:$dst), [(set i8:$dst, (load XYDispCP:$src))]> {
        bits<16> src;

        let Size = 4;
        field bits<32> Inst;
        let Inst{31 - 24} = 0x90;
        let Inst{23 - 16} = 0xD6;
        let Inst{15 - 0} = src;
    }

    def LDaispd8 : STM8Inst<"ld  \t$rd,$src", (ins DirectOff8SP:$src), (outs AREG:$rd), [(set i8:$rd, (load SPDispCP:$src))]> {
        bits<8> src;

        let Size = 2;
        field bits<16> Inst;
        let Inst{15 - 8} = 0x7B;
        let Inst{7 - 0} = src;

        let canFoldAsLoad = true;
    }

    def LDaim8 : STM8Inst<"ld  \t$rd,$src", (ins IndirectMem8:$src), (outs AREG:$rd), [(set i8:$rd, (load Imm8Pat:$src))]> {
        bits<8> src;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 16} = 0x92;
        let Inst{15 - 8} = 0xC6;
        let Inst{7 - 0} = src;
    }

    def LDaim16 : STM8Inst<"ld  \t$rd,$src", (ins IndirectMem16:$src), (outs AREG:$rd), [(set i8:$rd, (load Imm16Pat:$src))]> {
        bits<16> src;

        let Size = 4;
        field bits<32> Inst;
        let Inst{31 - 24} = 0x72;
        let Inst{23 - 16} = 0xC6;
        let Inst{15 - 0} = src;
    }

    def PseudoLDbispd8 : STM8Pseudo<"ld  \t$dst,$src", (ins DirectOff8SP:$src), (outs BREG:$dst)>;
}

//==============================================================================
// Stores to memory
//==============================================================================

let mayStore = true in {
    def LDm8a : STM8Inst<"ld  \t$dst,$src", (ins DirectMem8:$dst, AREG:$src), (outs), [(store i8:$src, Imm8Pat:$dst)]> {
        bits<8> dst;

        let Size = 2;
        field bits<16> Inst;
        let Inst{15 - 8} = 0xB7;
        let Inst{7 - 0} = dst;
    }

    def LDm16a : STM8Inst<"ld  \t$dst,$src", (ins DirectMem16:$dst, AREG:$src), (outs), [(store i8:$src, Imm16Pat:$dst)]> {
        bits<16> dst;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 16} = 0xC7;
        let Inst{15 - 0} = dst;
    }

    def LDixa : STM8Inst<"ld  \t$dst,$src", (ins DirectIdxX:$dst, AREG:$src), (outs), [(store i8:$src, i16:$dst)]> {
        let Size = 1;
        field bits<8> Inst = 0xF7;
    }

    def LDixd8a : STM8Inst<"ld  \t$dst,$src", (ins DirectOff8IdxX:$dst, AREG:$src), (outs), [(store i8:$src, XYDispCP:$dst)]> {
        bits<8> dst;

        let Size = 2;
        field bits<16> Inst;
        let Inst{15 - 8} = 0xE7;
        let Inst{7 - 0} = dst;
    }

    def LDixd16a : STM8Inst<"ld  \t$dst,$src", (ins DirectOff16IdxX:$dst, AREG:$src), (outs), [(store i8:$src, XYDispCP:$dst)]> {
        bits<16> dst;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 16} = 0xD7;
        let Inst{15 - 0} = dst;
    }

    def LDiya : STM8Inst<"ld  \t$dst,$src", (ins DirectIdxY:$dst, AREG:$src), (outs), [(store i8:$src, i16:$dst)]> {
        let Size = 2;
        field bits<16> Inst = 0x90F7;
    }

    def LDiyd8a : STM8Inst<"ld  \t$dst,$src", (ins DirectOff8IdxY:$dst, AREG:$src), (outs), [(store i8:$src, XYDispCP:$dst)]> {
        bits<8> dst;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 8} = 0x90E7;
        let Inst{7 - 0} = dst;
    }

    def LDiyd16a : STM8Inst<"ld  \t$dst,$src", (ins DirectOff16IdxY:$dst, AREG:$src), (outs), [(store i8:$src, XYDispCP:$dst)]> {
        bits<16> dst;

        let Size = 4;
        field bits<32> Inst;
        let Inst{31 - 16} = 0x90D7;
        let Inst{15 - 0} = dst;
    }

    def LDispd8a : STM8Inst<"ld  \t$dst,$src", (ins DirectOff8SP:$dst, AREG:$src), (outs), [(store i8:$src, SPDispCP:$dst)]> {
        bits<8> dst;

        let Size = 2;
        field bits<16> Inst;
        let Inst{15 - 8} = 0x6B;
        let Inst{7 - 0} = dst;
    }

    def LDim8a : STM8Inst<"ld  \t$dst,$src", (ins IndirectMem8:$dst, AREG:$src), (outs), [(store i8:$src, (load Imm8Pat:$dst))]> {
        bits<8> dst;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 8} = 0x92C7;
        let Inst{7 - 0} = dst;
    }

    def LDim16a : STM8Inst<"ld  \t$dst,$src", (ins IndirectMem16:$dst, AREG:$src), (outs), [(store i8:$src, (load Imm16Pat:$dst))]> {
        bits<16> dst;

        let Size = 4;
        field bits<32> Inst;
        let Inst{31 - 16} = 0x72C7;
        let Inst{15 - 0} = dst;
    }
    
    def PseudoLDispd8b : STM8Pseudo<"ld  \t$dst,$src", (ins DirectOff8SP:$dst, BREG:$src), (outs)>;
}

//==============================================================================
// Copies between A and X/Y sub-registers
// These copies do not set CC flags.
//==============================================================================

class LDrr8<STM8RegClass DstRC, STM8RegClass SrcRC, list<bits<8>> Enc>
    : STM8Inst<"ld  \t$dst,$src", (ins SrcRC:$src), (outs DstRC:$dst), []> {
    let Size = !size(Enc);
    field bits<16> Inst;
    let Inst{15 - 8} = !if(!eq(!size(Enc), 2), Enc[0], 0);
    let Inst{7 - 0} = !if(!eq(!size(Enc), 2), Enc[1], Enc[0]);
}

def LDxla : LDrr8<XLREG, AREG, [0x97]>;
def LDaxl : LDrr8<AREG, XLREG, [0x9F]>;
def LDyla : LDrr8<YLREG, AREG, [0x90, 0x97]>;
def LDayl : LDrr8<AREG, YLREG, [0x90, 0x9F]>;
def LDxha : LDrr8<XHREG, AREG, [0x95]>;
def LDaxh : LDrr8<AREG, XHREG, [0x9E]>;
def LDyha : LDrr8<YHREG, AREG, [0x90, 0x95]>;
def LDayh : LDrr8<AREG, YHREG, [0x90, 0x9E]>;

// Generic instructions for copying A to low/high part of X or Y
let Constraints = "$dst = $src_word" in {
    def GenLDrla : STM8GenInst<"ldlo\t$dst,$src_lo",
        (ins XY:$src_word, AREG:$src_lo), (outs XY:$dst),
        [(set i16:$dst, (STM8SetByte i16:$src_word, (i8 0), i8:$src_lo))]>;

    def GenLDrha : STM8GenInst<"ldhi\t$dst,$src_hi",
        (ins XY:$src_word, AREG:$src_hi), (outs XY:$dst),
        [(set i16:$dst, (STM8SetByte i16:$src_word, (i8 1), i8:$src_hi))]>;
}

// Generic instructions for copying low/high part of X or Y to A
def GenLDarl : STM8GenInst<"ldlo\t$dst,$src",
    (ins XY:$src), (outs AREG:$dst),
    [(set i8:$dst, (STM8GetByte i16:$src, (i8 0)))]>;

def GenLDarh : STM8GenInst<"ldhi\t$dst,$src",
    (ins XY:$src), (outs AREG:$dst),
    [(set i8:$dst, (STM8GetByte i16:$src, (i8 1)))]>;

//==============================================================================
// Copies to/from imaginary registers
//==============================================================================

let Defs = [CCn, CCz] in {
    // Copy between A and B
    def PseudoLDab : STM8Pseudo<"ld \t$dst,$src",
        (ins BREG:$src), (outs AREG:$dst)>;
    def PseudoLDba : STM8Pseudo<"ld \t$dst,$src",
        (ins AREG:$src), (outs BREG:$dst)>;

    // Copy between X/Y and Z
    def GenLDWrz : STM8GenInst<"ldw \t$dst,$src",
        (ins ZREG:$src), (outs XY:$dst)>;
    def GenLDWzr : STM8GenInst<"ldw\t$dst,$src",
        (ins XY:$src), (outs ZREG:$dst)>;
}

//==============================================================================
// MOV instructions
//==============================================================================

let mayStore = true in {
    def MOVm16i8 : STM8Inst<"mov \t$dst,$src", (ins DirectMem16:$dst, Imm8:$src), (outs), [(store Imm8Pat:$src, Imm16Pat:$dst)]> {
        bits<8> src;
        bits<16> dst;

        let Size = 4;
        field bits<32> Inst;
        let Inst{31 - 24} = 0x35;
        let Inst{23 - 16} = src;
        let Inst{15 - 0} = dst;
    }

    let mayLoad = true in {
        def MOVm8m8 : STM8Inst<"mov \t$dst,$src", (ins DirectMem8:$dst, DirectMem8:$src), (outs), [(store i8:$src, Imm8Pat:$dst)]> {
            bits<8> src;
            bits<8> dst;

            let Size = 3;
            field bits<24> Inst;
            let Inst{23 - 16} = 0x45;
            let Inst{15 - 8} = src;
            let Inst{7 - 0} = dst;
        }

        def MOVm16m16 : STM8Inst<"mov \t$dst,$src", (ins DirectMem16:$dst, DirectMem16:$src), (outs), [(store (load Imm16Pat:$src), Imm16Pat:$dst)]> {
            bits<16> src;
            bits<16> dst;

            let Size = 5;
            field bits<40> Inst;
            let Inst{39 - 32} = 0x55;
            let Inst{31 - 16} = src;
            let Inst{15 - 0} = dst;
        }
    }
}

//==============================================================================
// EXG/EXGW: Exchange registers
//==============================================================================

class ExgReg_Fmt<string mnemonic, DAGOperand LHS, DAGOperand RHS, int encSize>
    : STM8Inst<mnemonic # "\t$lhs,$rhs",
    (ins LHS:$lhs, RHS:$rhs), (outs LHS:$olhs, RHS:$orhs)> {
    let Constraints = "$olhs = $lhs, $orhs = $rhs";

    let Size = encSize;
    field bits<32> Inst;
}

def EXGaxl : ExgReg_Fmt<"exg ", AREG, XLREG, 1> { let Inst = 0x41; }
def EXGayl : ExgReg_Fmt<"exg ", AREG, YLREG, 1> { let Inst = 0x61; }

def EXGam16 : ExgReg_Fmt<"exg ", AREG, DirectMem16, 3> {
    bits<16> rhs;
    let Inst{23 - 16} = 0x31;
    let Inst{15 - 0} = rhs;

    let mayLoad = true;
    let mayStore = true;
}

def EXGWxy : ExgReg_Fmt<"exgw", XREG, YREG, 1> { let Inst = 0x51; }

//==============================================================================
// SWAP/SWAPW: Swap nibbles/bytes
//==============================================================================

class SwapFmtN<DAGOperand Op, bits<16> opCode, int encSrcBytes>
    : STM8Inst<"swap\t$src", (ins Op:$src), (outs)> {
    let Defs = [CCn, CCc];

    let Size = !add(!if(!eq(opCode{15 - 8}, 0), 1, 2), encSrcBytes);
    field bits<32> Inst;
}

class SwapFmt0<DAGOperand Op, bits<16> opCode> : SwapFmtN<Op, opCode, 0> {
    let Inst{15 - 0} = opCode;
}

class SwapFmt8<DAGOperand Op, bits<16> opCode> : SwapFmtN<Op, opCode, 1> {
    bits<8> src;
    let Inst{23 - 8} = opCode;
    let Inst{7 - 0} = src;
}

class SwapFmt16<DAGOperand Op, bits<16> opCode> : SwapFmtN<Op, opCode, 2> {
    bits<16> src;
    let Inst{31 - 16} = opCode;
    let Inst{15 - 0} = src;
}


def SWAPa : SwapFmt0<AREG, 0x4e> {
    let OutOperandList = (outs AREG:$dst);
    let Constraints = "$dst = $src";
}

let mayLoad = true, mayStore = true in {
    def SWAPm8    : SwapFmt8<DirectMem8, 0x3e>;
    def SWAPm16   : SwapFmt16<DirectMem16, 0x725e>;
    def SWAPix    : SwapFmt0<DirectIdxX, 0x7e>;
    def SWAPixd8  : SwapFmt8<DirectOff8IdxX, 0x6e>;
    def SWAPixd16 : SwapFmt16<DirectOff16IdxX, 0x724e>;
    def SWAPiy    : SwapFmt0<DirectIdxY, 0x907e>;
    def SWAPiyd8  : SwapFmt8<DirectOff8IdxY, 0x906e>;
    def SWAPiyd16 : SwapFmt16<DirectOff16IdxY, 0x904e>;
    def SWAPispd8 : SwapFmt8<DirectOff8SP, 0x0e>;
    def SWAPim8   : SwapFmt8<IndirectMem8, 0x923e>;
    def SWAPim16  : SwapFmt16<IndirectMem16, 0x723e>;
}

let Defs = [CCn, CCc], Constraints = "$dst = $src" in {
    // SWAPWx, SWAPWy
    foreach RC = [XREG, YREG] in {
        def "SWAPW" # RC.RS : STM8Inst<"swapw\t$src", (ins RC:$src), (outs RC:$dst)> {
            let Size = !if(!eq(RC, XREG), 1, 2);
            field bits<16> Inst = 0x905e;
        }
    }

    def GenSWAPWr : STM8GenInst<"swapw\t$src", (ins XY:$src), (outs XY:$dst),
        [(set i16:$dst, (STM8SwapBytes i16:$src))]>;
}

//==============================================================================
// Patterns for loads/stores from/to global variables
//==============================================================================

foreach AddrOp = [tglobaladdr, texternalsym] in {
    // 8 bit values
    def : Pat<(i8 (load (STM8Wrapper AddrOp:$src))), (LDam16 AddrOp:$src)>;
    def : Pat<(i8 (load (load (STM8Wrapper AddrOp:$src)))), (LDaim16 AddrOp:$src)>;
    def : Pat<(store i8:$src, (STM8Wrapper AddrOp:$dst)), (LDm16a AddrOp:$dst, AREG:$src)>;

    // 16 bit values. Instructions for Y reg do not exist.
    def : Pat<(i16 (load (STM8Wrapper AddrOp:$src))), (GenLDWrm16 AddrOp:$src)>;
    def : Pat<(i16 (load (load (STM8Wrapper AddrOp:$src)))), (LDWxim16 AddrOp:$src)>;
    def : Pat<(store i16:$src, (STM8Wrapper AddrOp:$dst)), (GenLDWm16r AddrOp:$dst, XY:$src)>;

    def : Pat<(i16 (STM8Wrapper AddrOp:$src)), (GenLDWri16 AddrOp:$src)>;
}
