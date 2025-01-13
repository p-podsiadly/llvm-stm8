; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:

    adc a, #0x55
    adc a, 0x10
    adc a, 0x1000
    adc a, (x)
    adc a, (#0x10,x)
    adc a, (#0x1000,x)
    adc a, (y)
    adc a, (#0x10,y)
    adc a, (#0x1000,y)
    adc a, (#0x10,sp)
    adc a, [0x10]
    adc a, [0x1000]

    ; TODO: indirect indexed addressing is not yet implemented
    ; adc a, ([0x10],x)
    ; adc a, ([0x1000],x)
    ; adc a, ([0x10],y)

; CHECK: adc a,#0x55        ; encoding: [0xa9,0x55]

; TODO: Asm matcher picks larger instruction with longmem operand.
; Fix this using MCOperandPredicate & AsmOperandClass.
; CHECK: adc a,0x10         ; encoding: [0xc9,0x00,0x10]
; CHECK: adc a,0x1000       ; encoding: [0xc9,0x10,0x00]
; CHECK: adc a,(x)          ; encoding: [0xf9]
; CHECK: adc a,(#0x10,x)    ; encoding: [0xd9,0x00,0x10]  
; CHECK: adc a,(#0x1000,x)  ; encoding: [0xd9,0x10,0x00]
; CHECK: adc a,(y)          ; encoding: [0x90,0xf9]
; CHECK: adc a,(#0x10,y)    ; encoding: [0x90,0xd9,0x00,0x10]
; CHECK: adc a,(#0x1000,y)  ; encoding: [0x90,0xd9,0x10,0x00]
; CHECK: adc a,(#0x10,sp)   ; encoding: [0x19,0x10]
; CHECK: adc a,[0x10]       ; encoding: [0x72,0xc9,0x00,0x10]
; CHECK: adc a,[0x1000]     ; encoding: [0x72,0xc9,0x10,0x00]
