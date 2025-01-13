// RUN: %clang_cc1 -triple stm8-unknown-unknown -emit-llvm -DCHECK_IR < %s| FileCheck %s
// RUN: %clang_cc1 %s -triple stm8-unknown-unknown -verify -fsyntax-only

#ifdef CHECK_IR

long stm8_default_cc(int a, char b) {
    return 0;
}

__attribute__((sdcccall0))
long stm8_sdcc_v0_cc(int a, char b) {
    return 0;
}

__attribute__((sdcccall1))
long stm8_sdcc_v1_cc(int a, char b) {
    return 0;
}

// CHECK: define dso_local i32 @stm8_default_cc
// CHECK: define dso_local sdcccall0 i32 @stm8_sdcc_v0_cc
// CHECK: define dso_local sdcccall1 i32 @stm8_sdcc_v1_cc

#else // CHECK_IR

__attribute__((sdcccall0))
long stm8_sdcc_v0_cc(int a, char b) {
    return 0;
}

__attribute__((sdcccall1))
long stm8_sdcc_v1_cc(int a, char b) {
    return 0;
}

__attribute__((sdcccall0)) int global_1; // expected-warning {{'sdcccall0' only applies to function types; type here is 'int'}}
__attribute__((sdcccall1)) int global_2; // expected-warning {{'sdcccall1' only applies to function types; type here is 'int'}}

#endif // CHECK_IR
