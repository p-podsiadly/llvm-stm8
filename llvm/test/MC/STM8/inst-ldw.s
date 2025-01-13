; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    ;;;;;;;;;;;;;;;
    ; Load into X ;
    ;;;;;;;;;;;;;;;

    LDW X, #0x1020
    LDW X, 0x10
    LDW X, 0x1020
    LDW X, (X)
    LDW X, (#0x10,X)
    LDW X, (#0x1020,X)
    LDW X, (#0x10,SP)
    LDW X, [0x10]
    LDW X, [0x1020]

; CHECK: ldw x,#0x1020          ; encoding: [0xae,0x10,0x20]
; CHECK: ldw x,0x10             ; encoding: [0xce,0x00,0x10]
; CHECK: ldw x,0x1020           ; encoding: [0xce,0x10,0x20]
; CHECK: ldw x,(x)              ; encoding: [0xfe]
; CHECK: ldw x,(#0x10,x)        ; encoding: [0xde,0x00,0x10]
; CHECK: ldw x,(#0x1020,x)      ; encoding: [0xde,0x10,0x20]
; CHECK: ldw x,(#0x10,sp)       ; encoding: [0x1e,0x10]
; CHECK: ldw x,[0x10]           ; encoding: [0x72,0xce,0x00,0x10]
; CHECK: ldw x,[0x1020]         ; encoding: [0x72,0xce,0x10,0x20]

    ;;;;;;;;;;;;;;;
    ; Load into Y ;
    ;;;;;;;;;;;;;;;

    LDW Y, #0x1020
    LDW Y, 0x10
    LDW Y, 0x1020
    LDW Y, (Y)
    LDW Y, (#0x10,Y)
    LDW Y, (#0x1020,Y)
    LDW Y, (#0x10,SP)
    LDW Y, [0x10]

; CHECK: ldw y,#0x1020          ; encoding: [0x90,0xae,0x10,0x20]
; CHECK: ldw y,0x10             ; encoding: [0x90,0xce,0x00,0x10]
; CHECK: ldw y,0x1020           ; encoding: [0x90,0xce,0x10,0x20]
; CHECK: ldw y,(y)              ; encoding: [0x90,0xfe]
; CHECK: ldw y,(#0x10,y)        ; encoding: [0x90,0xde,0x00,0x10]
; CHECK: ldw y,(#0x1020,y)      ; encoding: [0x90,0xde,0x10,0x20]
; CHECK: ldw y,(#0x10,sp)       ; encoding: [0x16,0x10]
; CHECK: ldw y,[0x10]           ; encoding: [0x91,0xce,0x10]

    ;;;;;;;;;;;;;;;;;
    ; Stores from X ;
    ;;;;;;;;;;;;;;;;;

    LDW 0x10,        X
    LDW 0x1020,      X
    LDW (Y),         X
    LDW (#0x10,Y),   X
    LDW (#0x1020,Y), X
    LDW (#0x10,SP),  X
    LDW [0x10],      X
    LDW [0x1020],    X

; CHECK: ldw 0x10,x             ; encoding: [0xcf,0x00,0x10]
; CHECK: ldw 0x1020,x           ; encoding: [0xcf,0x10,0x20]
; CHECK: ldw (y),x              ; encoding: [0x90,0xff]
; CHECK: ldw (#0x10,y),x        ; encoding: [0x90,0xdf,0x00,0x10]
; CHECK: ldw (#0x1020,y),x      ; encoding: [0x90,0xdf,0x10,0x20]
; CHECK: ldw (#0x10,sp),x       ; encoding: [0x1f,0x10]
; CHECK: ldw [0x10],x           ; encoding: [0x72,0xcf,0x00,0x10]
; CHECK: ldw [0x1020],x         ; encoding: [0x72,0xcf,0x10,0x20]

    ;;;;;;;;;;;;;;;;;
    ; Stores from Y ;
    ;;;;;;;;;;;;;;;;;

    LDW 0x10,        Y
    LDW 0x1020,      Y
    LDW (X),         Y
    LDW (#0x10,X),   Y
    LDW (#0x1020,X), Y
    LDW (#0x10,SP),  Y
    LDW [0x10],      Y

; CHECK: ldw 0x10,y             ; encoding: [0x90,0xcf,0x00,0x10]
; CHECK: ldw 0x1020,y           ; encoding: [0x90,0xcf,0x10,0x20]
; CHECK: ldw (x),y              ; encoding: [0xff]
; CHECK: ldw (#0x10,x),y        ; encoding: [0xdf,0x00,0x10]
; CHECK: ldw (#0x1020,x),y      ; encoding: [0xdf,0x10,0x20]
; CHECK: ldw (#0x10,sp),y       ; encoding: [0x17,0x10]
; CHECK: ldw [0x10],y           ; encoding: [0x91,0xcf,0x10]

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Copies between registers ;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    LDW Y,  X
    LDW X,  Y
    LDW X,  SP
    LDW SP, X
    LDW Y,  SP
    LDW SP, Y

; CHECK: ldw y,x                ; encoding: [0x90,0x93]
; CHECK: ldw x,y                ; encoding: [0x93]
; CHECK: ldw x,sp               ; encoding: [0x96]
; CHECK: ldw sp,x               ; encoding: [0x94]
; CHECK: ldw y,sp               ; encoding: [0x90,0x96]
; CHECK: ldw sp,y               ; encoding: [0x90,0x94]
