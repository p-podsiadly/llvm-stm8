; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:
    
    CP A, #0x10
    CP A, 0x10
    CP A, 0x1000
    CP A, (X)
    CP A, (#0x10,X)
    CP A, (#0x1000,X)
    CP A, (Y)
    CP A, (#0x10,Y)
    CP A, (#0x1000,Y)
    CP A, (#0x10,SP)
    CP A, [0x10]
    CP A, [0x1000]

    ; TODO indirect indexed with offset
    ; CP A, ([0x10],X)
    ; CP A, ([0x1000],X)
    ; CP A, ([0x10],Y)

; CHECK: cp a,#0x10         ; encoding: [0xa1,0x10]
; TODO short mem address
; CHECK: cp a,0x10          ; encoding: [0xc1,0x00,0x10]
; CHECK: cp a,0x1000        ; encoding: [0xc1,0x10,0x00]
; CHECK: cp a,(x)           ; encoding: [0xf1]
; TODO short offset
; CHECK: cp a,(#0x10,x)     ; encoding: [0xd1,0x00,0x10]
; CHECK: cp a,(#0x1000,x)   ; encoding: [0xd1,0x10,0x00]
; CHECK: cp a,(y)           ; encoding: [0x90,0xf1]
; TODO short offset
; CHECK: cp a,(#0x10,y)     ; encoding: [0x90,0xd1,0x00,0x10]
; CHECK: cp a,(#0x1000,y)   ; encoding: [0x90,0xd1,0x10,0x00]
; CHECK: cp a,(#0x10,sp)    ; encoding: [0x11,0x10]
; TODO short mem address
; CHECK: cp a,[0x10]        ; encoding: [0x72,0xc1,0x00,0x10]
; CHECK: cp a,[0x1000]      ; encoding: [0x72,0xc1,0x10,0x00]
