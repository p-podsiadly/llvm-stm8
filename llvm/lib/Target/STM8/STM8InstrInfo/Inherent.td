/* This file defines instructions using inherent addressing mode, i. e.
 * instructions without explicit operands.
 */

class Inherent<bits<8> opCodePrefix, bits<8> opCode, string asmStr,
               list<dag> pattern = []>
    : STM8Inst<asmStr, (ins), (outs), pattern> {
  field bits<16> Inst;

  // Instruction size: 1 byte if the prefix is 0, otherwise 2
  let Size = !cond(!ne(opCodePrefix, 0x00) : 2, true : 1);

  let Inst{15 - 8} = opCodePrefix;
  let Inst{7 - 0} = opCode;
}

def BREAK : Inherent<0x00, 0x8B, "break">;
def NOP : Inherent<0x00, 0x9D, "nop">;
def WFE : Inherent<0x72, 0x8F, "wfe">;

let Defs = [CCi] in {
  def WFI : Inherent<0x00, 0x8F, "wfi">;
  def HALT : Inherent<0x00, 0x8E, "halt">;
}

def IRET : Inherent<0x00, 0x80, "iret", [(STM8InterruptRetGlue)]> {
  let isTerminator = true;
  let isReturn = true;
  let isBarrier = true;

  let Uses = [SP];
  let Defs = [CCv, CCi, CCh, CCn, CCz, CCc, A, X, Y, SP];
}

let isTerminator = 1, isReturn = 1, isBarrier = 1, Uses = [SP], Defs = [SP] in {
  def RET : Inherent<0x00, 0x81, "ret", [(STM8RetGlue)]>;
  def RETF : Inherent<0x00, 0x87, "retf">;
}

// Set/reset carry flag
let Defs = [CCc] in {
  def SCF : Inherent<0x00, 0x99, "scf">;
  def RCF : Inherent<0x00, 0x98, "rcf">;
}

// Reset overflow flag
let Defs = [CCv] in {
  def RVF : Inherent<0x00, 0x9C, "rvf">;
}

// Set/reset interrupt mask
let Defs = [CCi] in {
  def SIM : Inherent<0x00, 0x9B, "sim">;
  def RIM : Inherent<0x00, 0x9A, "rim">;
}

def TRAP : Inherent<0x00, 0x83, "trap"> { let isTrap = true; }
