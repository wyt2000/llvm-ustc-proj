#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/IteratedDominanceFrontier.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>
#include <set>

#define tab(n) for(int i = 0; i < n; i++) llvm::outs() << "\t";

using namespace llvm;

#define DEBUG_TYPE "loop-search"
// STATISTIC(LoopSearched, "Number of loops has been found");

namespace llvm {
  FunctionPass * createLSPass();
  void initializeLSPassPass(PassRegistry&);
}

namespace {

struct LSPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  LSPass() : FunctionPass(ID) {
    initializeLSPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;
    size_t BackEdgeNum = 0;
    size_t LoopDepth   = 0;
    std::vector<size_t> LoopID;
		LoopID.push_back(0);
		
    DominatorTree DT(F);
    PostDominatorTree PDT(F);
    std::vector<DomTreeNode*> WorkList;
    WorkList.push_back(DT.getRootNode());
    llvm::outs() << "{\n";
    while(!WorkList.empty()){
      auto CurNode = WorkList.back();
      WorkList.pop_back();
      auto BB = CurNode->getBlock();
    	//llvm::outs() << "======== " << BB->getName() << " ========" << "\n";

      for(auto sPred = pred_begin(BB), ePred = pred_end(BB);sPred != ePred; ++sPred){
        auto PredNode = DT.getNode(*sPred);
        //llvm::outs() << "pred: " << PredNode->getBlock()->getName() << "\n";
        if(DT.dominates(CurNode, PredNode)){
					BackEdgeNum++;
          LoopDepth++;
          if(LoopID.size() <= LoopDepth) {
						LoopID.push_back(0);
					}
					LoopID[LoopDepth - 1]++;
					tab(LoopDepth);
					llvm::outs() << "\"L";
					for(int i = 0; i < LoopDepth; i++) {
						llvm::outs() << LoopID[i];
					}
					llvm::outs() << "\" : {\n";
					tab(LoopDepth + 1);
					llvm::outs() << "\"depth\" : " << LoopDepth << "\n";
        }
      }

      for(auto sSucc = succ_begin(BB), eSucc = succ_end(BB);sSucc != eSucc; ++sSucc){
        auto SuccNode = DT.getNode(*sSucc);
        //llvm::outs() << "succ: " << SuccNode->getBlock()->getName() << "\n";
        if(DT.dominates(SuccNode, CurNode)){
					LoopID[LoopDepth] = 0;
					tab(LoopDepth);
					llvm::outs() << "}\n";
          LoopDepth--;
        }
      }

      std::vector<DomTreeNode*> v = std::vector<DomTreeNode*>(CurNode->begin(), CurNode->end());
      std::reverse(v.begin(), v.end());
      WorkList.insert(WorkList.end(), v.begin(), v.end());
    }
    llvm::outs() << "}\n";
    llvm::outs() << "Processing function " << F.getName() << ", number of Backedges is " << BackEdgeNum << "\n";
    std::error_code err;
    raw_fd_ostream outfile_dt(StringRef(F.getName().str() + "_dt.txt"), err);
    raw_fd_ostream outfile_pdt(StringRef(F.getName().str() + "_pdt.txt"), err);
    DT.print(outfile_dt);
    PDT.print(outfile_pdt);
    return true;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<PostDominatorTreeWrapperPass>();
    AU.setPreservesCFG();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<PostDominatorTreeWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }
};

} // end anonymous namespace

char LSPass::ID = 0;
INITIALIZE_PASS(LSPass, "LSPass", "Loop search", false, false)

FunctionPass *llvm::createLSPass() { return new LSPass(); }