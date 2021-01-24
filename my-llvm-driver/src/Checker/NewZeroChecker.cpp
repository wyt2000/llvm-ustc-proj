#include "Checker/NewZeroChecker.h"
#include <iostream>
#include <string>
#include <math.h>
using namespace clang;
using namespace ento;
using llvm::APSInt;

// check whether the argument of malloc is negative.
// if so, report the error
void NewZeroChecker::checkNewAllocator(const CXXAllocatorCall &Call,
                                  CheckerContext &C) const {
  // check whether the function called is malloc
  //if( Call.getKindAsString() == "new" ) {
    std::cout << "There is new" << std::endl;
    // get Symbolic value of argument
    /*const Expr *arg1 = Call.getArgExpr(0);
    if(Call.getArgExpr(0) == nullptr)
        std::cout << "0 is null" << std::endl;
    std::cout << Call.getNumArgs() << std::endl;
    std::cout << Call.getNumImplicitArgs() << std::endl;
    std::cout << "1" << std::endl;
    SVal Argument = C.getSVal(arg1);*/
    const CXXNewExpr *Origin = nullptr;
    Origin = Call.getOriginExpr();
    const Expr *init = Origin->getInitializer();
    if (!init)
      std::cout << "no init" << std::endl;
    //SVal Argument = Call.getObjectUnderConstruction();
    SVal Argument = C.getSVal(init);
    std::cout << "2" << std::endl;
    if(Argument.isZeroConstant()) {
      std::cout<<"zero constant"<<std::endl;
      return;
    }
    std::cout<< "subkind" << Argument.getSubKind() <<std::endl;
    Optional<Loc> NL = Argument.getAs<Loc>();
    std::cout << "3" << std::endl;
    // return as the argument is not a concrete int
    if(!NL || NL->getSubKind() != loc::ConcreteIntKind){
      if(!NL) std::cout << "no NL" << std::endl;
        return;
    }
    std::cout << "4" << std::endl;
    // get the bit width of argument
    const auto Value = NL->castAs<loc::ConcreteInt>().getValue();
    std::cout << "5" << std::endl;
    unsigned Width =  Value.getBitWidth();
    ConstraintManager &CM = C.getConstraintManager();
    std::cout << "6" << std::endl;
    // two kinds of programstate
    /*ProgramStateRef stateNonPositive, statePositive;
    // whether the argument is unsigned or not
    bool isUnsigned = Value.isUnsigned();
    // check whether the argument is positive
    std::cout << "7" << std::endl;
    std::tie(stateNonPositive, statePositive)
      = CM.assumeInclusiveRangeDual(C.getState(), 
          *NL,
          llvm::APSInt(llvm::APInt(Width, 1), isUnsigned),
          llvm::APSInt(llvm::APInt(Width, pow(2,Width-1) - 1),isUnsigned) );
    std::cout << "8" << std::endl;
    // the argument is not positive,report this error
    if (!stateNonPositive) {
      assert(statePositive);
      // reset bugtype
      if (!BT) {
        BT.reset(new BugType(this,
          "argument in malloc should be positive","Example checker "));
      }
      ExplodedNode *N = C.generateErrorNode();
      auto Report = std::make_unique<PathSensitiveBugReport>(*BT,
                    "argument in malloc should be positive",N);
      // report explicitly
      C.emitReport(std::move(Report));
    }*/
    std::cout << "9" << std::endl;
  //} 
}
