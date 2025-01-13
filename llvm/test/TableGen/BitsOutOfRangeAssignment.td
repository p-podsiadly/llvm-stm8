// RUN: not llvm-tblgen %s 2>&1 | FileCheck %s

class TestClass<bit outOfRange> {
    field bits<16> Bits;
    let Bits{23 - 16} = !cond(outOfRange: 0x1234, true: 0x12);
    let Bits{15 - 0} = 0xabcd;
}

def Good : TestClass<false>;
def Bad  : TestClass<true>;

; CHECK: error: Initializer exceeds the size of the field
