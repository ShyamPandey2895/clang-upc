// RUN: %clang_cc1 %s -emit-llvm -triple x86_64-pc-linux -o - | FileCheck %s

shared int * testadd(shared int * ptr, int x) { return ptr + x; }
// CHECK: testadd
// CHECK: %2 = extractvalue %__upc_shared_pointer_type %0, 0
// CHECK-NEXT: %3 = and i64 %2, 1048575
// CHECK-NEXT: %4 = extractvalue %__upc_shared_pointer_type %0, 0
// CHECK-NEXT: %5 = lshr i64 %4, 20
// CHECK-NEXT: %6 = and i64 %5, 1023
// CHECK-NEXT: %7 = extractvalue %__upc_shared_pointer_type %0, 0
// CHECK-NEXT: %8 = lshr i64 %7, 30
// CHECK-NEXT: %idx.ext = sext i32 %1 to i64
// CHECK-NEXT: %9 = load i32* @THREADS
// CHECK-NEXT: %10 = zext i32 %9 to i64
// CHECK-NEXT: %11 = mul nuw i64 %10, 1
// CHECK-NEXT: %12 = mul nuw i64 %6, 1
// CHECK-NEXT: %13 = add nuw i64 %12, %3
// CHECK-NEXT: %14 = add i64 %13, %idx.ext
// CHECK-NEXT: %15 = sdiv i64 %14, %11
// CHECK-NEXT: %16 = srem i64 %14, %11
// CHECK-NEXT: %17 = icmp slt i64 %16, 0
// CHECK-NEXT: %18 = add i64 %16, %11
// CHECK-NEXT: %19 = select i1 %17, i64 %18, i64 %16
// CHECK-NEXT: %20 = sub i64 %15, 1
// CHECK-NEXT: %21 = select i1 %17, i64 %20, i64 %15
// CHECK-NEXT: %22 = udiv i64 %19, 1
// CHECK-NEXT: %23 = urem i64 %19, 1
// CHECK-NEXT: %24 = mul i64 %21, 1
// CHECK-NEXT: %25 = sub i64 %23, %3
// CHECK-NEXT: %26 = add i64 %25, %24
// CHECK-NEXT: %27 = mul i64 %26, 32
// CHECK-NEXT: %28 = add i64 %8, %27
// CHECK-NEXT: %29 = shl i64 %22, 20
// CHECK-NEXT: %30 = or i64 %29, %23
// CHECK-NEXT: %31 = shl i64 %28, 30
// CHECK-NEXT: %32 = or i64 %31, %30
// CHECK-NEXT: %33 = insertvalue %__upc_shared_pointer_type undef, i64 %32, 0

shared int * testsub(shared int * ptr, int x) { return ptr - x; }
// CHECK: testsub
// CHECK: %0 = load %__upc_shared_pointer_type* %ptr.addr, align 8
// CHECK-NEXT: %1 = load i32* %x.addr, align 4
// CHECK-NEXT: %2 = extractvalue %__upc_shared_pointer_type %0, 0
// CHECK-NEXT: %3 = and i64 %2, 1048575
// CHECK-NEXT: %4 = extractvalue %__upc_shared_pointer_type %0, 0
// CHECK-NEXT: %5 = lshr i64 %4, 20
// CHECK-NEXT: %6 = and i64 %5, 1023
// CHECK-NEXT: %7 = extractvalue %__upc_shared_pointer_type %0, 0
// CHECK-NEXT: %8 = lshr i64 %7, 30
// CHECK-NEXT: %idx.ext = sext i32 %1 to i64
// CHECK-NEXT: %9 = sub i64 0, %idx.ext
// CHECK-NEXT: %10 = load i32* @THREADS
// CHECK-NEXT: %11 = zext i32 %10 to i64
// CHECK-NEXT: %12 = mul nuw i64 %11, 1
// CHECK-NEXT: %13 = mul nuw i64 %6, 1
// CHECK-NEXT: %14 = add nuw i64 %13, %3
// CHECK-NEXT: %15 = add i64 %14, %9
// CHECK-NEXT: %16 = sdiv i64 %15, %12
// CHECK-NEXT: %17 = srem i64 %15, %12
// CHECK-NEXT: %18 = icmp slt i64 %17, 0
// CHECK-NEXT: %19 = add i64 %17, %12
// CHECK-NEXT: %20 = select i1 %18, i64 %19, i64 %17
// CHECK-NEXT: %21 = sub i64 %16, 1
// CHECK-NEXT: %22 = select i1 %18, i64 %21, i64 %16
// CHECK-NEXT: %23 = udiv i64 %20, 1
// CHECK-NEXT: %24 = urem i64 %20, 1
// CHECK-NEXT: %25 = mul i64 %22, 1
// CHECK-NEXT: %26 = sub i64 %24, %3
// CHECK-NEXT: %27 = add i64 %26, %25
// CHECK-NEXT: %28 = mul i64 %27, 32
// CHECK-NEXT: %29 = add i64 %8, %28
// CHECK-NEXT: %30 = shl i64 %23, 20
// CHECK-NEXT: %31 = or i64 %30, %24
// CHECK-NEXT: %32 = shl i64 %29, 30
// CHECK-NEXT: %33 = or i64 %32, %31
// CHECK-NEXT: %34 = insertvalue %__upc_shared_pointer_type undef, i64 %33, 0

long long testsub2(shared int * ptr1, shared int * ptr2) { return ptr1 - ptr2; }
// CHECK: testsub2
// CHECK: %0 = load %__upc_shared_pointer_type* %ptr1.addr, align 8
// CHECK-NEXT: %1 = load %__upc_shared_pointer_type* %ptr2.addr, align 8
// CHECK-NEXT: %2 = extractvalue %__upc_shared_pointer_type %0, 0
// CHECK-NEXT: %3 = and i64 %2, 1048575
// CHECK-NEXT: %4 = extractvalue %__upc_shared_pointer_type %0, 0
// CHECK-NEXT: %5 = lshr i64 %4, 20
// CHECK-NEXT: %6 = and i64 %5, 1023
// CHECK-NEXT: %7 = extractvalue %__upc_shared_pointer_type %0, 0
// CHECK-NEXT: %8 = lshr i64 %7, 30
// CHECK-NEXT: %9 = extractvalue %__upc_shared_pointer_type %1, 0
// CHECK-NEXT: %10 = and i64 %9, 1048575
// CHECK-NEXT: %11 = extractvalue %__upc_shared_pointer_type %1, 0
// CHECK-NEXT: %12 = lshr i64 %11, 20
// CHECK-NEXT: %13 = and i64 %12, 1023
// CHECK-NEXT: %14 = extractvalue %__upc_shared_pointer_type %1, 0
// CHECK-NEXT: %15 = lshr i64 %14, 30
// CHECK-NEXT: %addr.diff = sub i64 %8, %15
// CHECK-NEXT: %16 = sdiv exact i64 %addr.diff, 32
// CHECK-NEXT: %17 = load i32* @THREADS
// CHECK-NEXT: %18 = zext i32 %17 to i64
// CHECK-NEXT: %thread.diff = sub i64 %6, %13
// CHECK-NEXT: %19 = mul i64 %thread.diff, 1
// CHECK-NEXT: %phase.diff = sub i64 %3, %10
// CHECK-NEXT: %20 = sub i64 %16, %phase.diff
// CHECK-NEXT: %block.diff = mul i64 %20, %18
// CHECK-NEXT: %21 = mul i64 %19, %phase.diff
// CHECK-NEXT: %ptr.diff = add i64 %block.diff, %21

shared int *testsubscript(shared int * ptr, int idx) { return &ptr[idx]; }
// CHECK: testsubscript
// CHECK: %0 = load i32* %idx.addr, align 4
// CHECK-NEXT: %idxprom = sext i32 %0 to i64
// CHECK-NEXT: %1 = load %__upc_shared_pointer_type* %ptr.addr, align 8
// CHECK-NEXT: %2 = extractvalue %__upc_shared_pointer_type %1, 0
// CHECK-NEXT: %3 = and i64 %2, 1048575
// CHECK-NEXT: %4 = extractvalue %__upc_shared_pointer_type %1, 0
// CHECK-NEXT: %5 = lshr i64 %4, 20
// CHECK-NEXT: %6 = and i64 %5, 1023
// CHECK-NEXT: %7 = extractvalue %__upc_shared_pointer_type %1, 0
// CHECK-NEXT: %8 = lshr i64 %7, 30
// CHECK-NEXT: %9 = load i32* @THREADS
// CHECK-NEXT: %10 = zext i32 %9 to i64
// CHECK-NEXT: %11 = mul nuw i64 %10, 1
// CHECK-NEXT: %12 = mul nuw i64 %6, 1
// CHECK-NEXT: %13 = add nuw i64 %12, %3
// CHECK-NEXT: %14 = add i64 %13, %idxprom
// CHECK-NEXT: %15 = sdiv i64 %14, %11
// CHECK-NEXT: %16 = srem i64 %14, %11
// CHECK-NEXT: %17 = icmp slt i64 %16, 0
// CHECK-NEXT: %18 = add i64 %16, %11
// CHECK-NEXT: %19 = select i1 %17, i64 %18, i64 %16
// CHECK-NEXT: %20 = sub i64 %15, 1
// CHECK-NEXT: %21 = select i1 %17, i64 %20, i64 %15
// CHECK-NEXT: %22 = udiv i64 %19, 1
// CHECK-NEXT: %23 = urem i64 %19, 1
// CHECK-NEXT: %24 = mul i64 %21, 1
// CHECK-NEXT: %25 = sub i64 %23, %3
// CHECK-NEXT: %26 = add i64 %25, %24
// CHECK-NEXT: %27 = mul i64 %26, 32
// CHECK-NEXT: %28 = add i64 %8, %27
// CHECK-NEXT: %29 = shl i64 %22, 20
// CHECK-NEXT: %30 = or i64 %29, %23
// CHECK-NEXT: %31 = shl i64 %28, 30
// CHECK-NEXT: %32 = or i64 %31, %30
// CHECK-NEXT: %33 = insertvalue %__upc_shared_pointer_type undef, i64 %32, 0
