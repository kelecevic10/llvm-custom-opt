cmake_minimum_required(VERSION 3.13)
project(LLVMCustomOpt LANGUAGES C CXX)

# Putanja do LLVM instalacije (build-a)
set(LLVM_DIR "/home/nemanja/Desktop/Konstrukcija Kompilatora/LLVM/llvm-project/build/lib/cmake/llvm")

# Ucitaj LLVM konfiguraciju
find_package(LLVM REQUIRED CONFIG)
set(TERMINFO_LIB "")  # iskljuci Terminfo proveru

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Ucitaj LLVM-ove makroe (ukljucuje add_llvm_library)
include(${LLVM_DIR}/AddLLVM.cmake)

# Uključi LLVM zaglavlja i definicije
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Genericki plugin (test primer)
add_llvm_library(MyDeadCodeEliminationPass MODULE
        src/MyDeadCodeEliminationPass.cpp
        src/MyCFG.cpp

        PLUGIN_TOOL
        opt
)
