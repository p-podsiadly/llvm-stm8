// Copy Carry Bit to Memory
def BCCMm16 : STM8Inst<"bccm\t$dst,$pos", (ins DirectMem16:$dst, Imm8:$pos), (outs), []> {
    bits<16> dst;
    bits<8> pos;

    let Size = 4;
    
    field bits<32> Inst;
    let Inst{31 - 24} = 0x90;
    let Inst{23 - 20} = 0x1;
    let Inst{19 - 17} = pos{2 - 0};
    let Inst{16 - 16} = 0x1;
    let Inst{15 - 0} = dst;

    let mayLoad = true;
    let mayStore = true;

    let Uses = [CCc];
}

// Bit Complement
def BCPLm16 : STM8Inst<"bcpl\t$dst,$pos", (ins DirectMem16:$dst, Imm8:$pos), (outs), []> {
    bits<16> dst;
    bits<8> pos;

    let Size = 4;

    field bits<32> Inst;
    let Inst{31 - 24} = 0x90;
    let Inst{23 - 20} = 0x1;
    let Inst{19 - 17} = pos{2 - 0};
    let Inst{16 - 16} = 0x0;
    let Inst{15 - 0} = dst;

    let mayLoad = true;
    let mayStore = true;
}

// Bit Reset
def BRESm16 : STM8Inst<"bres\t$dst,$pos", (ins DirectMem16:$dst, Imm8:$pos), (outs), []> {
    bits<16> dst;
    bits<8> pos;

    let Size = 4;

    field bits<32> Inst;
    let Inst{31 - 24} = 0x72;
    let Inst{23 - 20} = 0x1;
    let Inst{19 - 17} = pos{2 - 0};
    let Inst{16 - 16} = 0x1;
    let Inst{15 - 0} = dst;

    let mayLoad = true;
    let mayStore = true;
}

// Bit Set
def BSETm16 : STM8Inst<"bset\t$dst,$pos", (ins DirectMem16:$dst, Imm8:$pos), (outs), []> {
    bits<16> dst;
    bits<8> pos;

    let Size = 4;

    field bits<32> Inst;
    let Inst{31 - 24} = 0x72;
    let Inst{23 - 20} = 0x1;
    let Inst{19 - 17} = pos{2 - 0};
    let Inst{16 - 16} = 0x0;
    let Inst{15 - 0} = dst;

    let mayLoad = true;
    let mayStore = true;
}

// Bit Test and Jump if False
def BTJFm16 : STM8Inst<"btjf\t$addr,$pos,$target",
    (ins DirectMem16:$addr, Imm8:$pos, PCRel8:$target), (outs), []> {
    bits<16> addr;
    bits<8> pos;
    bits<8> target;

    let Size = 5;

    field bits<40> Inst;
    let Inst{39 - 32} = 0x72;
    let Inst{31 - 28} = 0x0;
    let Inst{27 - 25} = pos{2 - 0};
    let Inst{24 - 24} = 0x1;
    let Inst{23 - 8} = addr;
    let Inst{7 - 0} = target;

    let mayLoad = true;
    let isBranch = true;
    let isTerminator = true;
}

// Bit Test and Jump if True
def BTJTm16 : STM8Inst<"btjt\t$addr,$pos,$target",
    (ins DirectMem16:$addr, Imm8:$pos, PCRel8:$target), (outs), []> {
    bits<16> addr;
    bits<8> pos;
    bits<8> target;

    let Size = 5;

    field bits<40> Inst;
    let Inst{39 - 32} = 0x72;
    let Inst{31 - 28} = 0x0;
    let Inst{27 - 25} = pos{2 - 0};
    let Inst{24 - 24} = 0x0;
    let Inst{23 - 8} = addr;
    let Inst{7 - 0} = target;

    let mayLoad = true;
    let isBranch = true;
    let isTerminator = true;
}
