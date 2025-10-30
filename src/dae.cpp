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

// samo da vidimo kad se .so učita
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
      // ako nije Instruction (konzervativno), tretiraj kao pravu upotrebu
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

/// Klonira F u NF bez mrtvih argumenata.
/// Ključne stvari:
///  - ne kopiramo AttributeList/metadata → izbegnemo context mismatch
///  - za mrtve argumente u VMap stavljamo UndefValue::get(ArgTy) → CloneFunctionInto ne puca
static Function* cloneWithoutDeadArgs(Function &F, ArrayRef<unsigned> liveIdxs) {
  Module *M = F.getParent();

  // set živih argumenata
  llvm::SmallSet<unsigned, 8> liveSet;
  for (unsigned v : liveIdxs) liveSet.insert(v);

  // novi tip: samo živi parametri
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

  // nemoj kopirati atribute/metadata; samo pozivna konvencija i to je to
  NF->setCallingConv(F.getCallingConv());

  // VMap: mapiramo SAMO žive argumente -> nove argumente
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

  // kloniraj telo; napomena: dozvoli module-level promene
  SmallVector<ReturnInst*, 8> Returns;
  CloneFunctionInto(NF, &F, VMap, /*ModuleLevelChanges*/true, Returns);

  return NF;
}

/// Prevezuje SVE pozive OldF → NewF sa filtriranim argumentima.
/// Ne prepisuje atribute/operand bundles; ubacuje novi poziv PRE starog.
static void rewriteCallSites(Function &OldF, Function &NewF,
                             ArrayRef<unsigned> liveIdxs) {
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
    // sve ostalo (npr. uzeta adresa) filtriramo u runOnFunction
  }
}

struct DeadArgumentElimination : public FunctionPass {
  static char ID;
  DeadArgumentElimination() : FunctionPass(ID) {}

bool runOnFunction(Function &F) override {
  errs() << "[DAE] Running on function: " << F.getName() << "\n"; errs().flush();
  if (F.isDeclaration()) return false;

  // ako postoji ijedan user koji NIJE call/invoke, preskačemo (adresataken)
  for (User *U : F.users())
    if (!isa<CallInst>(U) && !isa<InvokeInst>(U))
      return false;

  if (!hasDeadArgs(F)) return false;

  std::vector<unsigned> liveIdxs = getLiveArgIdxs(F);
  Function *NewF = cloneWithoutDeadArgs(F, liveIdxs);

  // Preveži sve pozive na novu funkciju
  rewriteCallSites(F, *NewF, liveIdxs);
  // zamena imena bez brisanja F:
  std::string OldName = F.getName().str();
  F.setName(OldName + ".old");  // staru skloni u .old
  NewF->setName(OldName);       // nova preuzima staro ime


  // >>> KLJUČNA PROMENA: NE BRIŠI F <<<
  // Umesto eraseFromParent(), samo isprazni telo da ostane validna deklaracija.
  while (!F.empty())
    F.begin()->eraseFromParent();
  // (opciono) F.setLinkage(GlobalValue::ExternalLinkage); // svakako je deklaracija
  // (opciono) F.clearMetadata(); // nije neophodno

  errs() << "[DAE] Removed dead args in: " << OldName << "\n";
  return true;
}
};

} // end anonymous namespace

char DeadArgumentElimination::ID = 0;
static RegisterPass<DeadArgumentElimination>
X("dae-pass", "DAE pass", false, false);
