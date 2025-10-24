//
// Created by nemanja on 10/24/25.
//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct MyDeadCodeEliminationPass : public FunctionPass {
        static char ID;
        MyDeadCodeEliminationPass() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            errs() << "[dead-code-elimination-pass] Running on function: " << F.getName() << "\n";
            return false;
        }
    };
}

char MyDeadCodeEliminationPass::ID = 0;
static RegisterPass<MyDeadCodeEliminationPass> X("dead-code-elimination-pass", "MyDeadCodeEliminationPass", false, false);