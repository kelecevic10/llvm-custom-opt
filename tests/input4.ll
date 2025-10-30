; ModuleID = 'tests/input4.cpp'
source_filename = "tests/input4.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; sum(a,b): koristi oba argumenta
define dso_local i32 @sum(i32 %0, i32 %1) #0 {
entry:
  %2 = add nsw i32 %0, %1
  ret i32 %2
}

; product(x,y): koristi oba argumenta
define dso_local i32 @product(i32 %0, i32 %1) #0 {
entry:
  %2 = mul nsw i32 %0, %1
  ret i32 %2
}

; main poziva sum i product, koristi rezultate
define dso_local i32 @main() #1 {
entry:
  %0 = call i32 @sum(i32 2, i32 3)
  %1 = call i32 @product(i32 %0, i32 4)
  %2 = add nsw i32 %1, 1
  ret i32 %2
}

attributes #0 = { noinline nounwind optnone uwtable }
attributes #1 = { noinline nounwind optnone uwtable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 10.0.1"}
