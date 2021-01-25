#include "Checker/NewZeroChecker.h"
#include <iostream>
#include <string>
#include <math.h>
using namespace clang;
using namespace ento;
using llvm::APSInt;
SimpleProgramPointTag tag("H2020.NewZeroChecker", "new argument should be positive");

// check whether the argument of new is not positive.
// if so, report the error
void NewZeroChecker::checkNewAllocator(const CXXAllocatorCall &Call,
                                  CheckerContext &C) const {
    unsigned implicitargs = Call.getNumImplicitArgs();
    const Expr * arg1=nullptr;
    if (const CallExpr *CE = dyn_cast<CallExpr>(Call.getOriginExpr())) {
     arg1 = CE->getArg(0);
    } 
    else if (const CXXNewExpr *NE = dyn_cast<CXXNewExpr>(Call.getOriginExpr())) {
      if (NE->isArray()) {
        arg1 = *NE->getArraySize();
      } 
      else {
        return;
      }
    }
    else {
      llvm_unreachable("not a CallExpr or CXXNewExpr");
    }

    // get symbolic value of argument
    SVal Argument = C.getSVal(arg1);
    Optional<NonLoc> NL = Argument.getAs<NonLoc>();
    const auto Value = NL->castAs<nonloc::ConcreteInt>().getValue();
    unsigned Width =  Value.getBitWidth();
    ConstraintManager &CM = C.getConstraintManager();

    // set program state
    ProgramStateRef stateNonPositive, statePositive;
    bool isUnsigned = Value.isUnsigned();
    std::tie(stateNonPositive, statePositive)
      = CM.assumeInclusiveRangeDual(C.getState(), 
          *NL,
          llvm::APSInt(llvm::APInt(Width, 1), isUnsigned),
          llvm::APSInt(llvm::APInt(Width, pow(2,Width-1) - 1),isUnsigned) );
          
    // the argument is not positive,report this error
    if (!stateNonPositive) {
      assert(statePositive);
      // reset bugtype
      if (!BT) {
        BT.reset(new BugType(this,
          "new argument should be positive","newZeroChecker"));
      }
      ExplodedNode *N = C.generateErrorNode(C.getState(), &tag);
      auto Report = std::make_unique<PathSensitiveBugReport>(*BT,
                    "new argument should be positive", N);
      // report explicitly
      C.emitReport(std::move(Report));
    }
}