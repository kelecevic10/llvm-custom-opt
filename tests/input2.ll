; ModuleID = 'tests/input2.cpp'
source_filename = "tests/input2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; helper(a, b): koristi samo a → b mrtav
define dso_local i32 @helper(i32 %0, i32 %1) #0 {
entry:
  %2 = mul nsw i32 %0, 2
  ret i32 %2
}

; compute(x, y, z): koristi x i z → y mrtav
define dso_local i32 @compute(i32 %0, i32 %1, i32 %2) #0 {
entry:
  %3 = call i32 @helper(i32 %0, i32 0)
  %4 = add nsw i32 %3, %2
  ret i32 %4
}

; main poziva compute(3,4,5)
define dso_local i32 @main() #1 {
entry:
  %0 = call i32 @compute(i32 3, i32 4, i32 5)
  ret i32 %0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 10.0.1"}
