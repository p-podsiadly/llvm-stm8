; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    ;;;;;;;;;;;;;;;;;;;;;;;
    ; X on left hand side ;
    ;;;;;;;;;;;;;;;;;;;;;;;

    CPW X, #0x1000
    CPW X, 0x10
    CPW X, 0x1000
    CPW X, (Y)
    CPW X, (#0x10,Y)
    CPW X, (#0x1000,Y)
    CPW X, (#0x10,SP)
    CPW X, [0x10]
    CPW X, [0x1000]

    ; TODO indirect indexed with offset
    ; CPW X, ([0x10],Y)

; CHECK: cpw x,#0x1000      ; encoding: [0xa3,0x10,0x00]
; TODO short mem address
; CHECK: cpw x,0x10         ; encoding: [0xc3,0x00,0x10]
; CHECK: cpw x,0x1000       ; encoding: [0xc3,0x10,0x00]
; CHECK: cpw x,(y)          ; encoding: [0x90,0xf3]
; TODO short offset
; CHECK: cpw x,(#0x10,y)    ; encoding: [0x90,0xd3,0x00,0x10]
; CHECK: cpw x,(#0x1000,y)  ; encoding: [0x90,0xd3,0x10,0x00]
; CHECK: cpw x,(#0x10,sp)   ; encoding: [0x13,0x10]
; TODO short mem address
; CHECK: cpw x,[0x10]       ; encoding: [0x72,0xc3,0x00,0x10]
; CHECK: cpw x,[0x1000]     ; encoding: [0x72,0xc3,0x10,0x00]

    ;;;;;;;;;;;;;;;;;;;;;;;
    ; Y on left hand side ;
    ;;;;;;;;;;;;;;;;;;;;;;;

    CPW Y, #0x1000
    CPW Y, 0x10
    CPW Y, 0x1000
    CPW Y, (X)
    CPW Y, (#0x10,X)
    CPW Y, (#0x1000,X)
    CPW Y, [0x10]

    ; TODO indirect indexed with offset
    ; CPW Y, ([0x10],X)
    ; CPW Y, ([0x1000],X)

; CHECK: cpw y,#0x1000      ; encoding: [0x90,0xa3,0x10,0x00]
; TODO short mem address
; CHECK: cpw y,0x10         ; encoding: [0x90,0xc3,0x00,0x10]
; CHECK: cpw y,0x1000       ; encoding: [0x90,0xc3,0x10,0x00]
; CHECK: cpw y,(x)          ; encoding: [0xf3]
; TODO short offset
; CHECK: cpw y,(#0x10,x)    ; encoding: [0xd3,0x00,0x10]
; CHECK: cpw y,(#0x1000,x)  ; encoding: [0xd3,0x10,0x00]
; CHECK: cpw y,[0x10]       ; encoding: [0x91,0xc3,0x10]
