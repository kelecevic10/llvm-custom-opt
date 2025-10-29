#!/bin/bash

# Path to LLVM build folder
LLVM_BUILD="/home/nemanja/Desktop/Konstrukcija Kompilatora/LLVM/llvm-project/build"

rm -f tests/*.ll
"$LLVM_BUILD/bin/clang" -emit-llvm -O0 -S tests/input.cpp -o tests/input.ll