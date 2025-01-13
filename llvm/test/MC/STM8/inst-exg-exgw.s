; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    exg a,xl
    exg a,yl
    exg a,0x1234

; CHECK: exg a,xl       ; encoding: [0x41]
; CHECK: exg a,yl       ; encoding: [0x61]
; CHECK: exg a,0x1234   ; encoding: [0x31,0x12,0x34]

    exgw x,y

; CHECK: exgw x,y       ; encoding: [0x51]
