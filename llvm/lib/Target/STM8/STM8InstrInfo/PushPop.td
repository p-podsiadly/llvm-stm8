//==============================================================================
// Push onto stack, pop from stack
//==============================================================================

def SDT_STM8Push : SDTypeProfile<0, 1, []>;
def SDT_STM8Pop  : SDTypeProfile<1, 0, []>;

def STM8Push : SDNode<"STM8::PUSH", SDT_STM8Push, [SDNPHasChain, SDNPMayStore]>;
def STM8Pop  : SDNode<"STM8::POP",  SDT_STM8Pop,  [SDNPHasChain, SDNPMayLoad]>;

let Uses = [SP], Defs = [SP], mayStore = true in {
    def PUSHa : STM8Inst<"push\t$reg", (ins AREG:$reg), (outs), [(STM8Push AREG:$reg)]> {
        let Size = 1;
        field bits<8> Inst = 0x88;
    }

    def PUSHi8 : STM8Inst<"push\t$val", (ins Imm8:$val), (outs), [(STM8Push Imm8Pat:$val)]> {
        bits<8> val;

        let Size = 2;
        field bits<16> Inst;
        let Inst{15 - 8} = 0x4B;
        let Inst{7 - 0} = val;
    }

    def PUSHm16 : STM8Inst<"push\t$src", (ins DirectMem16:$src), (outs), [(STM8Push (load i16:$src))]> {
        bits<16> src;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 16} = 0x3B;
        let Inst{15 - 0} = src;
    }

    def PUSHWx : STM8Inst<"pushw\t$reg", (ins XREG:$reg), (outs), [(STM8Push XREG:$reg)]> {
        let Size = 1;
        field bits<8> Inst = 0x89;
    }

    def PUSHWy : STM8Inst<"pushw\t$reg", (ins YREG:$reg), (outs), [(STM8Push YREG:$reg)]> {
        let Size = 2;
        field bits<16> Inst = 0x9089;
    }
}

def PUSHcc : STM8Inst<"push\t$cc", (ins CCREG:$cc), (outs), []> {
    let Size = 1;
    field bits<8> Inst = 0x8A;
    let Uses = [SP, CCv, CCi, CCh, CCn, CCz, CCc];
    let Defs = [SP];
}

let Uses = [SP], Defs = [SP], mayLoad = true in {
    def POPa : STM8Inst<"pop \t$reg", (ins), (outs AREG:$reg), [(set AREG:$reg, STM8Pop)]> {
        let Size = 1;
        field bits<8> Inst = 0x84;
    }

    def POPm16 : STM8Inst<"pop \t$dst", (ins DirectMem16:$dst), (outs), [(store STM8Pop, Imm16Pat:$dst)]> {
        bits<16> dst;

        let Size = 3;
        field bits<24> Inst;
        let Inst{23 - 16} = 0x32;
        let Inst{15 - 0} = dst;
    }

    def POPWx : STM8Inst<"popw\t$reg", (ins), (outs XREG:$reg), [(set XREG:$reg, STM8Pop)]> {
        let Size = 1;
        field bits<8> Inst = 0x85;
    }

    def POPWy : STM8Inst<"popw\t$reg", (ins), (outs YREG:$reg), [(set YREG:$reg, STM8Pop)]> {
        let Size = 2;
        field bits<16> Inst = 0x9085;
    }
}

def POPcc : STM8Inst<"pop \t$cc", (ins), (outs CCREG:$cc), []> {
    let Size = 1;
    field bits<8> Inst = 0x86;
    let Uses = [SP];
    let Defs = [SP, CCv, CCi, CCh, CCn, CCz, CCc];
}
