// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm -fsanitize=kcfi -o - %s | FileCheck %s --check-prefixes=CHECK,C
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm -fsanitize=kcfi -x c++ -o - %s | FileCheck %s --check-prefixes=CHECK,MEMBER
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm -fsanitize=kcfi -fpatchable-function-entry-offset=3 -o - %s | FileCheck %s --check-prefixes=CHECK,OFFSET
#if !__has_feature(kcfi)
#error Missing kcfi?
#endif

/// Must emit __kcfi_typeid symbols for address-taken function declarations
// CHECK: module asm ".weak __kcfi_typeid_[[F4:[a-zA-Z0-9_]+]]"
// CHECK: module asm ".set __kcfi_typeid_[[F4]], [[#%d,HASH:]]"
/// Must not __kcfi_typeid symbols for non-address-taken declarations
// CHECK-NOT: module asm ".weak __kcfi_typeid_{{f6|_Z2f6v}}"

// C: @ifunc1 = ifunc i32 (i32), ptr @resolver1
// C: @ifunc2 = ifunc i64 (i64), ptr @resolver2
typedef int (*fn_t)(void);

// CHECK: define dso_local{{.*}} i32 @{{f1|_Z2f1v}}(){{.*}} !kcfi_type ![[#TYPE:]]
int f1(void) { return 0; }

// CHECK: define dso_local{{.*}} i32 @{{f2|_Z2f2v}}(){{.*}} !kcfi_type ![[#TYPE2:]]
unsigned int f2(void) { return 2; }

// CHECK-LABEL: define dso_local{{.*}} i32 @{{__call|_Z6__callPFivE}}(ptr{{.*}} %f)
int __call(fn_t f) __attribute__((__no_sanitize__("kcfi"))) {
  // CHECK-NOT: call{{.*}} i32 %{{.}}(){{.*}} [ "kcfi"
  return f();
}

// CHECK: define dso_local{{.*}} i32 @{{call|_Z4callPFivE}}(ptr{{.*}} %f){{.*}}
int call(fn_t f) {
  // CHECK: call{{.*}} i32 %{{.}}(){{.*}} [ "kcfi"(i32 [[#HASH]]) ]
  return f();
}

#ifndef __cplusplus
// C: define internal ptr @resolver1() #[[#]] {
int ifunc1(int) __attribute__((ifunc("resolver1")));
static void *resolver1(void) { return 0; }

// C: define internal ptr @resolver2() #[[#]] {
static void *resolver2(void) { return 0; }
long ifunc2(long) __attribute__((ifunc("resolver2")));
#endif

// CHECK-DAG: define internal{{.*}} i32 @{{f3|_ZL2f3v}}(){{.*}} !kcfi_type ![[#TYPE]]
static int f3(void) { return 1; }

// CHECK-DAG: declare !kcfi_type ![[#TYPE]]{{.*}} i32 @[[F4]]()
extern int f4(void);

/// Must not emit !kcfi_type for non-address-taken local functions
// CHECK: define internal{{.*}} i32 @{{f5|_ZL2f5v}}()
// CHECK-NOT: !kcfi_type
// CHECK-SAME: {
static int f5(void) { return 2; }

// CHECK-DAG: declare !kcfi_type ![[#TYPE]]{{.*}} i32 @{{f6|_Z2f6v}}()
extern int f6(void);

int test(void) {
  return call(f1) +
         __call((fn_t)f2) +
         call(f3) +
         call(f4) +
         f5() +
         f6();
}

#ifdef __cplusplus
struct A {
  // MEMBER-DAG: define{{.*}} void @_ZN1A1fEv(ptr{{.*}} %this){{.*}} !kcfi_type ![[#TYPE3:]]
  void f() {}
};

void test_member_call(void) {
  void (A::* p)() = &A::f;
  // MEMBER-DAG: call void %[[#]](ptr{{.*}} [ "kcfi"(i32 [[#%d,HASH3:]]) ]
  (A().*p)();
}
#endif

// CHECK-DAG: ![[#]] = !{i32 4, !"kcfi", i32 1}
// OFFSET-DAG: ![[#]] = !{i32 4, !"kcfi-offset", i32 3}
// CHECK-DAG: ![[#TYPE]] = !{i32 [[#HASH]]}
// CHECK-DAG: ![[#TYPE2]] = !{i32 [[#%d,HASH2:]]}
// MEMBER-DAG: ![[#TYPE3]] = !{i32 [[#HASH3]]}
