; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    mul x, a
    mul y, a

; CHECK: mul x,a    ; encoding: [0x42]
; CHECK: mul y,a    ; encoding: [0x90,0x42]
