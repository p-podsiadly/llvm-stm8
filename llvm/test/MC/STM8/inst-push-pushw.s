; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    PUSH A
    PUSH CC
    PUSH #0x10
    PUSH 0x1000

; CHECK: push a         ; encoding: [0x88]
; CHECK: push cc        ; encoding: [0x8a]
; CHECK: push #0x10     ; encoding: [0x4b,0x10]
; CHECK: push 0x1000    ; encoding: [0x3b,0x10,0x00]

    PUSHW X
    PUSHW Y

; CHECK: pushw x        ; encoding: [0x89]
; CHECK: pushw y        ; encoding: [0x90,0x89]
