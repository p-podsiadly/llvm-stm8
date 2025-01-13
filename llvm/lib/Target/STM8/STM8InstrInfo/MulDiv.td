//==============================================================================
// Multiplication
//==============================================================================

let Defs = [CCh, CCc] in {
    foreach RC = [XREG, YREG] in {
        // MULxa, MULya
        def "MUL" # RC.RS # "a" : STM8Inst<"mul \t$dst,$rhs",
            (ins RC:$lhs, AREG:$rhs), (outs RC:$dst)> {
            let Constraints = "$dst = $lhs";

            let Size = !if(!eq(RC, XREG), 1, 2);
            field bits<16> Inst = 0x9042;
        }
    }

    def GenMULra : STM8GenInst<"mul\t$dst,$rhs",
        (ins XY:$lhs, AREG:$rhs), (outs XY:$dst)> {
        let Constraints = "$dst = $lhs";
    }
}

def : Pat<
    (mul i8:$lhs, i8:$rhs),
    (Trunc16to8 (GenMULra (AnyExt8to16 i8:$lhs), AREG:$rhs))>;

//==============================================================================
// Division
//==============================================================================

let Defs = [CCv, CCh, CCn, CCz, CCc] in {

    // DIV: divide X or Y by A

    foreach RC = [XREG, YREG] in {
        // DIVxa, DIVya
        def "DIV" # RC.RS # "a" : STM8Inst<"div \t$quot,$rhs",
            (ins RC:$lhs, AREG:$rhs), (outs RC:$quot, AREG:$rem)> {
            let Constraints = "$quot = $lhs, $rem = $rhs";

            let Size = !if(!eq(RC, XREG), 1, 2);
            field bits<16> Inst = 0x9062;
        }
    }

    def GenDIVra : STM8GenInst<"div\t$quot,$rhs",
        (ins XY:$lhs, AREG:$rhs), (outs XY:$quot, AREG:$rem)> {
        let Constraints = "$quot = $lhs, $rem = $rhs";
    }

    // DIVW: divide X by Y

    def DIVWxy : STM8Inst<"divw\t$lhs,$rhs",
        (ins XREG:$lhs, YREG:$rhs), (outs XREG:$quot, YREG:$rem)> {
        let Constraints = "$quot = $lhs, $rem = $rhs";

        let Size = 1;
        field bits<8> Inst = 0x65;
    }
}

def : Pat<(STM8UDivRem i16:$lhs, i8:$rhs), (GenDIVra XY:$lhs, AREG:$rhs)>;
def : Pat<(STM8UDivRem i16:$lhs, i16:$rhs), (DIVWxy XREG:$lhs, YREG:$rhs)>;
