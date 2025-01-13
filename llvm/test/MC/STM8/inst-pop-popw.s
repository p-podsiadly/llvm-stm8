; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    POP A
    POP CC
    POP 0x1000

; CHECK: pop a          ; encoding: [0x84]
; CHECK: pop cc         ; encoding: [0x86]
; CHECK: pop 0x1000     ; encoding: [0x32,0x10,0x00]

    POPW X
    POPW Y

; CHECK: popw x         ; encoding: [0x85]
; CHECK: popw y         ; encoding: [0x90,0x85]
