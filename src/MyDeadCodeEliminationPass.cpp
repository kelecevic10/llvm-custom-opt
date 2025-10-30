//
// Created by nemanja on 10/24/25.
//

#include "llvm/Pass.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "MyCFG.h"

using namespace llvm;

namespace {
    struct MyDeadCodeEliminationPass : public FunctionPass {

        std::unordered_map<Value *, bool> Variables;            // stores potentially dead instructions
        std::unordered_map<Value *, Value *> VariablesMap;
        std::set<Instruction *> InstructionsToRemove;
        bool InstructionEliminated;

        static char ID;
        MyDeadCodeEliminationPass() : FunctionPass(ID) {}

        void handleOperand(Value *Operand) {
            // the operand generally can be a constant, so we want to mark it live only if it is a variable
            if (Variables.find(Operand) != Variables.end())
                Variables[Operand] = true;

            if (VariablesMap.find(Operand) != VariablesMap.end())
                Variables[VariablesMap[Operand]] = true;
        }

        void EliminateDeadInstructions(Function &F) {

            InstructionsToRemove.clear();

            // for each var, save the last store instruction
            std::unordered_map<Value *, StoreInst *> LastStore;

            for (BasicBlock &BB: F) {
                for (Instruction &I: BB) {
                    if (auto *store = dyn_cast<StoreInst>(&I)) {
                        Value *ptr = store->getOperand(1);

                        // if the store instruction already exists for this pointer,
                        //  the old store instruction is dead
                        if (LastStore.find(ptr) != LastStore.end()) {
                            errs() << "[dse: " << F.getName() << "]: Dead store detected and removed -> " << *LastStore[ptr] << "\n";
                            InstructionsToRemove.insert(LastStore[ptr]);
                        }

                        LastStore[ptr] = store;
                    }
                    // load instruction means that the value is read, so we remove the LastStore active pointer
                    else if (auto *load = dyn_cast<LoadInst>(&I)) {
                        Value* ptr = load->getOperand(0);
                        LastStore.erase(ptr);
                    }
                }
            }

            // at the end of the pass above, all store instructions that remain
            // in the LastStore map are never used (there is no load instruction that uses them)
            for (auto& entry: LastStore) {
                InstructionsToRemove.insert(entry.second);
            }

            for (BasicBlock& BB: F) {
                for (Instruction& I: BB) {
                    // void and call instructions are not something we would like to eliminate
                    if (!I.getType()->isVoidTy() && !isa<CallInst>(&I)) {
                        Variables[&I] = false;
                    }

                    if (isa<LoadInst>(&I)) {
                        VariablesMap[&I] = I.getOperand(0);
                    }

                    if (isa<StoreInst>(&I)) {
                        handleOperand(I.getOperand(0));
                    }
                    else {                                                  // for every other instruction we just mark all of its operands as live
                        for (int i = 0; i < I.getNumOperands(); i++) {
                            handleOperand(I.getOperand(i));
                        }
                    }
                }
            }

            for (BasicBlock& BB: F) {
                for (Instruction& I: BB) {
                    // we delete store instruction only when it's first operand is a dead variable - it's result is not used
                    // for all the other instruction, we just delete it if it's result (stored in the &I pointer) is dead
                    if (isa<StoreInst>(&I) && Variables.find(I.getOperand(1)) != Variables.end() && !Variables[I.getOperand(1)]) {
                        InstructionsToRemove.insert(&I);
                    }
                    else {
                        if (Variables.find(&I) != Variables.end() && !Variables[&I]) {
                            InstructionsToRemove.insert(&I);
                        }
                    }
                }
            }

            if (!InstructionsToRemove.empty()) {
                InstructionEliminated = true;
            }

            for (Instruction* I: InstructionsToRemove) {
                errs() << "[dce: " << F.getName() << "]: Instruction " << *I << " deleted\n";
                I->eraseFromParent();
            }
        }

        void EliminateUnreachableInstructions(Function &F) {
            std::vector<BasicBlock *> UnreachableBlocks;
            MyCFG *CFG = new MyCFG(F);

            CFG->TraverseGraph();

            for (BasicBlock &BB : F) {
                if (!CFG->isReachable(&BB)) {
                    UnreachableBlocks.push_back(&BB);
                }
            }

            if (!UnreachableBlocks.empty()) {
                InstructionEliminated = true;
            }

            for (BasicBlock *UnreachableBlock : UnreachableBlocks) {
                errs() << "[dce]: block " << *UnreachableBlock << " eliminated\n";
                UnreachableBlock->eraseFromParent();
            }
        }



        bool runOnFunction(Function &F) override {
            // clear maps so old values (from the others funcs) don't influence current func info
            Variables.clear();
            VariablesMap.clear();
            InstructionsToRemove.clear();

            do {
                InstructionEliminated = false;
                EliminateDeadInstructions(F);
                EliminateUnreachableInstructions(F);
            } while (InstructionEliminated);

            return false;
        }
    };
}

char MyDeadCodeEliminationPass::ID = 0;
static RegisterPass<MyDeadCodeEliminationPass> X("dead-code-elimination-pass", "MyDeadCodeEliminationPass", false, false);