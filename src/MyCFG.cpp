//
// Created by nemanja on 10/26/25.
//

#include "MyCFG.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

MyCFG::MyCFG(Function &F) {
    CreateCFG(F);
}

void MyCFG::CreateCFG(Function &F) {
    // .front() returns the first basic block in the function
    StartBlock = &F.front();
    for (BasicBlock &BB : F) {
        AdjacencyList[&BB] = {};
        for (BasicBlock* Successor : successors(&BB)) {
            AdjacencyList[&BB].push_back(Successor);
        }
        EndBlock = &BB;
    }
}

void MyCFG::CreateTransposeCFG(Function &F) {
    for (BasicBlock &BB : F) {
        TransposeAdjacencyList[&BB] = {};
        for (BasicBlock* Predecessor : predecessors(&BB)) {
            TransposeAdjacencyList[&BB].push_back(Predecessor);
        }
    }
}

// starts dfs from the start block and fills the visited set
void MyCFG::TraverseGraph() {
    DFS(StartBlock);
}

void MyCFG::DFS(BasicBlock* Current) {
    Visited.insert(Current);
    for (BasicBlock* Successor : AdjacencyList[Current]) {
        if (Visited.find(Successor) == Visited.end()) {
            DFS(Successor);
        }
    }
}

// returns the reverse topological order of the CFG using BFS from the end block
std::vector<BasicBlock *> MyCFG::GetTraverseOrder() {
    std::queue<BasicBlock *> q;
    std::unordered_set<BasicBlock *> returnVal;

    q.push(EndBlock);
    returnVal.insert(EndBlock);

    while (!q.empty()) {
        BasicBlock *Current = q.front();
        q.pop();
        for (BasicBlock *BB : predecessors(Current)) {
            if (returnVal.find(BB) == returnVal.end()) {
                q.push(BB);
                returnVal.insert(BB);
            }
        }
    }

    std::vector<BasicBlock *> rv(returnVal.begin(), returnVal.end());
    reverse(rv.begin(), rv.end());
    return rv;
}

bool MyCFG::isReachable(BasicBlock *BB) {
    return Visited.find(BB) != Visited.end();
}