; RUN: llvm-mc -triple stm8 -show-encoding < %s | FileCheck %s

test:
    jra label_01

label_01:
    call ExternalFunc

; CHECK:      jra  label_01
; CHECK-NEXT: ; fixup A - offset: 1, value: label_01, kind: fixup_8_pcrel

; CHECK:      call ExternalFunc
; CHECK-NEXT: ; fixup A - offset: 1, value: ExternalFunc, kind: fixup_16
