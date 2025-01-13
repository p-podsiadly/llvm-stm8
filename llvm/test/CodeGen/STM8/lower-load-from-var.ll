; RUN: llc < %s -march=stm8 | FileCheck --match-full-lines %s

define i8 @test_i8(ptr %addr) {
    %res = load i8, ptr %addr
    ret i8 %res
}

; CHECK: ld a,(x)
; CHECK: ret

define i16 @test_i16(ptr %addr) {
    %res = load i16, ptr %addr
    ret i16 %res
}

; CHECK: ldw x,(x)
