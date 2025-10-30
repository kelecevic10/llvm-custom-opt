#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"   // DbgInfoIntrinsic
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

struct _DAEHello {
  _DAEHello() { errs() << "[DAE] Plugin loaded\n"; }
} _daehello_instance;

namespace {

/// Ignoriše dbg upotrebe; traži “prave” upotrebe.
static bool hasNonDebugUses(Value &V) {
  for (User *U : V.users()) {
    if (Instruction *I = dyn_cast<Instruction>(U)) {
      if (!isa<DbgInfoIntrinsic>(I)) return true;
    } else {
      return true;
    }
  }
  return false;
}

static bool hasDeadArgs(Function &F) {
  for (Argument &A : F.args())
    if (!hasNonDebugUses(A))
      return true;
  return false;
}

static std::vector<unsigned> getLiveArgIdxs(Function &F) {
  std::vector<unsigned> live;
  unsigned idx = 0;
  for (Argument &A : F.args()) {
    if (hasNonDebugUses(A)) live.push_back(idx);
    ++idx;
  }
  return live;
}

static Function* cloneWithoutDeadArgs(Function &F, ArrayRef<unsigned> liveIdxs) {
  Module *M = F.getParent();

  llvm::SmallSet<unsigned, 8> liveSet;
  for (unsigned v : liveIdxs) liveSet.insert(v);

  SmallVector<Type*, 8> paramTys;
  paramTys.reserve(liveIdxs.size());
  unsigned idx = 0;
  for (Argument &A : F.args()) {
    if (liveSet.count(idx))
      paramTys.push_back(A.getType());
    ++idx;
  }

  FunctionType *NFTy =
      FunctionType::get(F.getReturnType(), paramTys, F.isVarArg());

  // nova funkcija u ISTOM modulu/kontekstu
  Function *NF = Function::Create(NFTy, F.getLinkage(), F.getName() + ".dae", M);

  NF->setCallingConv(F.getCallingConv());

  ValueToValueMapTy VMap;
  auto NAI = NF->arg_begin();
  idx = 0;
  for (Argument &A : F.args()) {
    if (liveSet.count(idx)) {
      if (A.hasName()) NAI->setName(A.getName());
      VMap[&A] = &*NAI;
      ++NAI;
    }
    ++idx;
  }

  SmallVector<ReturnInst*, 8> Returns;
  CloneFunctionInto(NF, &F, VMap, /*ModuleLevelChanges*/true, Returns);

  return NF;
}

static void rewriteCallSites(Function &OldF, Function &NewF, ArrayRef<unsigned> liveIdxs) {
  SmallVector<User*, 16> Users(OldF.user_begin(), OldF.user_end());

  for (User *U : Users) {
    if (CallInst *CI = dyn_cast<CallInst>(U)) {
      SmallVector<Value*, 8> NewArgs;
      NewArgs.reserve(liveIdxs.size());
      for (unsigned li : liveIdxs) NewArgs.push_back(CI->getArgOperand(li));

      CallInst *NewCall = CallInst::Create(&NewF, NewArgs, "", CI);
      NewCall->setCallingConv(CI->getCallingConv());

      if (!CI->getType()->isVoidTy())
        CI->replaceAllUsesWith(NewCall);
      CI->eraseFromParent();

    } else if (InvokeInst *II = dyn_cast<InvokeInst>(U)) {
      SmallVector<Value*, 8> NewArgs;
      NewArgs.reserve(liveIdxs.size());
      for (unsigned li : liveIdxs) NewArgs.push_back(II->getArgOperand(li));

      InvokeInst *NewInvoke =
          InvokeInst::Create(&NewF, II->getNormalDest(), II->getUnwindDest(),
                             NewArgs, "", II);
      NewInvoke->setCallingConv(II->getCallingConv());

      if (!II->getType()->isVoidTy())
        II->replaceAllUsesWith(NewInvoke);
      II->eraseFromParent();
    }
  }
}

struct DeadArgumentElimination : public FunctionPass {
  static char ID;
  DeadArgumentElimination() : FunctionPass(ID) {}

bool runOnFunction(Function &F) override {
  errs() << "[DAE] Running on function: " << F.getName() << "\n"; errs().flush();
  if (F.isDeclaration()) return false;

  for (User *U : F.users())
    if (!isa<CallInst>(U) && !isa<InvokeInst>(U))
      return false;

  if (!hasDeadArgs(F)) return false;

  std::vector<unsigned> liveIdxs = getLiveArgIdxs(F);
  Function *NewF = cloneWithoutDeadArgs(F, liveIdxs);

  // Prevezujemo sve pozive na novu funkciju
  rewriteCallSites(F, *NewF, liveIdxs);
  // zamena imena bez brisanja F
  std::string OldName = F.getName().str();
  F.setName(OldName + ".old");  
  NewF->setName(OldName);       


  while (!F.empty())
    F.begin()->eraseFromParent();
    errs() << "[DAE] Removed dead args in: " << OldName << "\n";
  return true;
}
};

} 

char DeadArgumentElimination::ID = 0;
static RegisterPass<DeadArgumentElimination>
X("dae-pass", "DAE pass", false, false);
