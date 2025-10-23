#!/bin/bash

# Path to LLVM build folder
LLVM_BUILD="/home/nemanja/Desktop/Konstrukcija Kompilatora/LLVM/llvm-project/build"

"$LLVM_BUILD/bin/clang" -emit-llvm -S tests/input.cpp -o tests/input.ll

"$LLVM_BUILD/bin/opt" -load "$PWD/cmake-build-debug/LLVMExamplePass.so" -enable-new-pm=0 -example-pass tests/input.ll -o tests/output.ll -S
