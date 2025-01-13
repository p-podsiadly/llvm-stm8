; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    div x,a
    div y,a

; CHECK: div  x,a   ; encoding: [0x62]
; CHECK: div  y,a   ; encoding: [0x90,0x62]

    divw x,y

; CHECK: divw x,y   ; encoding: [0x65]
