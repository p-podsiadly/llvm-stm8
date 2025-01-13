; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    ;;;;;;;;;;;;;;;
    ; Load into A ;
    ;;;;;;;;;;;;;;;

    LD A, #0x55
    LD A, 0x50
    LD A, 0x5000
    LD A, (X)
    LD A, (#0x50,X)
    LD A, (#0x5000,X)
    LD A, (Y)
    LD A, (#0x50,Y)
    LD A, (#0x5000,Y)
    LD A, (#0x50,SP)
    LD A, [0x50]
    LD A, [0x5000]

    ; TODO indirect indexed addressing
    ;LD A, ([0x50],X)
    ;LD A, ([0x5000],X)
    ;LD A, ([0x50],Y)

; CHECK: ld a,#0x55         ; encoding: [0xa6,0x55]
; TODO: 0x50 should be treated as short mem addr:
; CHECK: ld a,0x50          ; encoding: [0xc6,0x00,0x50]
; CHECK: ld a,0x5000        ; encoding: [0xc6,0x50,0x00]
; CHECK: ld a,(x)           ; encoding: [0xf6]
; TODO: 0x50 should be treated as short offset:
; CHECK: ld a,(#0x50,x)     ; encoding: [0xd6,0x00,0x50]
; CHECK: ld a,(#0x5000,x)   ; encoding: [0xd6,0x50,0x00]
; CHECK: ld a,(y)           ; encoding: [0x90,0xf6]
; TODO: 0x50 should be treated as short offset:
; CHECK: ld a,(#0x50,y)     ; encoding: [0x90,0xd6,0x00,0x50]
; CHECK: ld a,(#0x5000,y)   ; encoding: [0x90,0xd6,0x50,0x00]
; CHECK: ld a,(#0x50,sp)    ; encoding: [0x7b,0x50]
; TODO: 0x50 should be treated as short mem addr:
; CHECK: ld a,[0x50]        ; encoding: [0x72,0xc6,0x00,0x50]
; CHECK: ld a,[0x5000]      ; encoding: [0x72,0xc6,0x50,0x00]
; TODO: 0x50 should be treated as short mem addr:
; TODO-CHECK: ld a,([0x50],x)    ; encoding: [0x72,0xd6,0x00,0x50]
; TODO-CHECK: ld a,([0x5000],x)  ; encoding: [0x72,0xd6,0x50,0x00]
; TODO-CHECK: ld a,([0x50],y)    ; encoding: [0x91,0xd6,0x50]

    ;;;;;;;;;;;;;;;;
    ; Store from A ;
    ;;;;;;;;;;;;;;;;

    LD 0x50, A
    LD 0x5000, A
    LD (X), A
    LD (#0x50,X), A
    LD (#0x5000,X), A
    LD (Y), A
    LD (#0x50,Y), A
    LD (#0x5000,Y), A
    LD (#0x50,SP), A
    LD [0x50], A
    LD [0x5000], A

    ; TODO indirect indexed addressing
    ; LD ([0x50],X), A
    ; LD ([0x5000],X), A
    ; LD ([0x50],Y), A

; TODO short mem address
; CHECK: ld 0x50,a          ; encoding: [0xc7,0x00,0x50]
; CHECK: ld 0x5000,a        ; encoding: [0xc7,0x50,0x00]
; CHECK: ld (x),a           ; encoding: [0xf7]
; TODO short offset
; CHECK: ld (#0x50,x),a     ; encoding: [0xd7,0x00,0x50]
; CHECK: ld (#0x5000,x),a   ; encoding: [0xd7,0x50,0x00]
; CHECK: ld (y),a           ; encoding: [0x90,0xf7]
; TODO short offset
; CHECK: ld (#0x50,y),a     ; encoding: [0x90,0xd7,0x00,0x50]
; CHECK: ld (#0x5000,y),a   ; encoding: [0x90,0xd7,0x50,0x00]
; CHECK: ld (#0x50,sp),a    ; encoding: [0x6b,0x50]
; TODO short mem address
; CHECK: ld [0x50],a        ; encoding: [0x72,0xc7,0x00,0x50]
; CHECK: ld [0x5000],a      ; encoding: [0x72,0xc7,0x50,0x00]

    ;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Copy between registers ;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; TODO A <-> half reg copies
    ; LD XL, A
    ; LD A, XL
    ; LD YL, A
    ; LD A, YL
    ; LD XH, A
    ; LD A, XH
    ; LD YH, A
    ; LD A, YH

; TODO-CHECK: ld xl,a            ; encoding: [0x97]
; TODO-CHECK: ld a,xl            ; encoding: [0x9f]
; TODO-CHECK: ld yl,a            ; encoding: [0x90,0x97]
; TODO-CHECK: ld a,yl            ; encoding: [0x90,0x9f]
; TODO-CHECK: ld xh,a            ; encoding: [0x95]
; TODO-CHECK: ld a,xh            ; encoding: [0x9e]
; TODO-CHECK: ld yh,a            ; encoding: [0x90,0x95]
; TODO-CHECK: ld a,yh            ; encoding: [0x90,0x9e]
