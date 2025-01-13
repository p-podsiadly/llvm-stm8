; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    sll a
    sll 0x15
    sll 0x1505
    ; TODO...

; CHECK: sll a           ; encoding: [0x48]
; CHECK: sll 0x15        ; encoding: [0x72,0x58,0x00,0x15]
; CHECK: sll 0x1505      ; encoding: [0x72,0x58,0x15,0x05]

    ; Mnemonic alias
    sla (#0x1234,y)
    sla [0x1234]

; CHECK: sll (#0x1234,y) ; encoding: [0x90,0x48,0x12,0x34]
; CHECK: sll [0x1234]    ; encoding: [0x72,0x38,0x12,0x34]

    sllw x
    sllw y

; CHECK: sllw x          ; encoding: [0x58]
; CHECK: sllw y          ; encoding: [0x90,0x58]

    ; Mnemonic alias
    slaw x
    slaw y

; CHECK: sllw x          ; encoding: [0x58]
; CHECK: sllw y          ; encoding: [0x90,0x58]
