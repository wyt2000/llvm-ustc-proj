#include "Checker/MallocOverflowSecurityChecker.h"
#include <iostream>
#include <string>
#include <math.h>
using namespace clang;
using namespace ento;
using llvm::APSInt;

// check whether the argument of malloc is negative.
// if so, report the error
void MallocOverflowSecurityChecker::checkPreStmt(
        const CallExpr *CE,CheckerContext &C) const {
  // check whether the function called is malloc
  if( CE->getDirectCallee()->getNameAsString() == "malloc" ) {
    // get Symbolic value of argument
    const Expr *arg1 = CE->getArg(0);
    SVal Argument = C.getSVal(arg1);
    Optional<NonLoc> NL = Argument.getAs<NonLoc>();
    // return as the argument is not a concrete int
    if(!NL || NL->getSubKind() != nonloc::ConcreteIntKind){
        return;
    }
    // get the bit width of argument
    const auto Value = NL->castAs<nonloc::ConcreteInt>().getValue();
    unsigned Width =  Value.getBitWidth();
    ConstraintManager &CM = C.getConstraintManager();
    // two kinds of programstate
    ProgramStateRef stateNonPositive, statePositive;
    // whether the argument is unsigned or not
    bool isUnsigned = Value.isUnsigned();
    // check whether the argument is positive
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
          "argument in malloc should be positive","MallocOverflowChecker"));
      }
      ExplodedNode *N = C.generateErrorNode();
      auto Report = std::make_unique<PathSensitiveBugReport>(*BT,
                    "argument in malloc should be positive",N);
      // report explicitly
      C.emitReport(std::move(Report));
    }
  } 
}
