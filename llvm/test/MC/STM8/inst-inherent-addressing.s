; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    BREAK
    HALT
    IRET
    NOP
    RCF
    RET
    RETF
    RIM
    RVF
    SCF
    SIM
    TRAP
    WFE
    WFI

; CHECK: break  ; encoding: [0x8b]
; CHECK: halt   ; encoding: [0x8e]
; CHECK: iret   ; encoding: [0x80]
; CHECK: nop    ; encoding: [0x9d]
; CHECK: rcf    ; encoding: [0x98]
; CHECK: ret    ; encoding: [0x81]
; CHECK: retf   ; encoding: [0x87]
; CHECK: rim    ; encoding: [0x9a]
; CHECK: rvf    ; encoding: [0x9c]
; CHECK: scf    ; encoding: [0x99]
; CHECK: sim    ; encoding: [0x9b]
; CHECK: trap   ; encoding: [0x83]
; CHECK: wfe    ; encoding: [0x72,0x8f]
; CHECK: wfi    ; encoding: [0x8f]