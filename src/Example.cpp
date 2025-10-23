#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct ExamplePass : public FunctionPass {
        static char ID;
        ExamplePass() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            errs() << "[ExamplePass] Running on function: " << F.getName() << "\n";
            return false;
        }
    };
}

char ExamplePass::ID = 0;
static RegisterPass<ExamplePass> X("example-pass", "Example Pass", false, false);