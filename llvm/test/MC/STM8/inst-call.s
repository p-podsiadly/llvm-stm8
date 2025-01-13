; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    CALL 0x1000
    CALL (X)
    CALL (#0x10,X)
    CALL (#0x1000,X)
    CALL (Y)
    CALL (#0x10,Y)
    CALL (#0x1000,Y)
    CALL [0x10]
    CALL [0x1000]

    ; TODO
    ; CALL ([0x10],X)
    ; CALL ([0x1000],X)
    ; CALL ([0x10],Y)

; CHECK: call   0x1000      ; encoding: [0xcd,0x10,0x00]
; CHECK: call   (x)         ; encoding: [0xfd]

; TODO asm parser interprets this as long offset instead of short offset
; FIXME-CHECK: call   (#0x10,x)   ; encoding: [0xed,0x10]
; CHECK: call   (#0x10,x)   ; encoding: [0xdd,0x00,0x10]

; CHECK: call   (#0x1000,x) ; encoding: [0xdd,0x10,0x00]
; CHECK: call   (y)         ; encoding: [0x90,0xfd]

; TODO same as above
; FIXME-CHECK: call   (#0x10,y)   ; encoding: [0x90,0xed,0x10]
; CHECK: call   (#0x10,y)   ; encoding: [0x90,0xdd,0x00,0x10]

; CHECK: call   (#0x1000,y) ; encoding: [0x90,0xdd,0x10,0x00]

; TODO same as above
; FIXME-CHECK: call   [0x10]      ; encoding: [0x92,0xcd,0x10]
; CHECK: call   [0x10]      ; encoding: [0x72,0xcd,0x00,0x10]

; CHECK: call   [0x1000]    ; encoding: [0x72,0xcd,0x10,0x00]
