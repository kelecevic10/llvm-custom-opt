//
// Created by nemanja on 10/26/25.
//

#ifndef LLVMCUSTOMOPT_MYCFG_H
#define LLVMCUSTOMOPT_MYCFG_H

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/CFG.h"
#include <unordered_map>
#include <vector>
#include <unordered_set>

using namespace llvm;

class MyCFG {

private:
    // the list of successors for each basic block
    std::unordered_map<BasicBlock *, std::vector<BasicBlock *>> AdjacencyList;
    // the list of predecessors for each basic block
    std::unordered_map<BasicBlock *, std::vector<BasicBlock *>> TransposeAdjacencyList;
    // the set of visited basic blocks
    std::unordered_set<BasicBlock *> Visited;
    BasicBlock* StartBlock;
    BasicBlock* EndBlock;

    void CreateCFG(Function &F);
    void DFS(BasicBlock* Current);

public:
    MyCFG(Function &F);
    std::vector<BasicBlock *> GetTraverseOrder();
    void CreateTransposeCFG(Function &F);
    void TraverseGraph();
    bool isReachable(BasicBlock *);
};


#endif //LLVMCUSTOMOPT_MYCFG_H
