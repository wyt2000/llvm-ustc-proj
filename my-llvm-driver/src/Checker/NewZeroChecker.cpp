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


    // std::cout<<"a"<<std::endl;
    // const Expr *arg1 = Call->getArg(0);
    // std::cout<<"b"<<std::endl;
    // SVal Argument = C.getSVal(arg1);
    // std::cout<<"c"<<std::endl;
    // Optional<NonLoc> NL = Argument.getAs<NonLoc>();
    // std::cout<<"d"<<std::endl;
    // // return as the argument is not a concrete int
    // if(!NL || NL->getSubKind() != nonloc::ConcreteIntKind){
    //   std::cout<<"e"<<std::endl;
    //     return;
    // }
    // // get the bit width of argument
    // const auto Value = NL->castAs<nonloc::ConcreteInt>().getValue();
    // std::cout<<"f"<<std::endl;
    // // std::cout<<Value<<std::endl;
    // std::cout<<"g"<<std::endl;
    unsigned implicitargs = Call.getNumImplicitArgs();
    std::cout << "implicitargs" << implicitargs << std::endl;
    // const CallExpr *CE = dyn_cast<CallExpr>(Call.getOriginExpr());
    const Expr * arg1=nullptr;
       if (const CallExpr *CE = dyn_cast<CallExpr>(Call.getOriginExpr())) {
     arg1 = CE->getArg(0);
   } else if (const CXXNewExpr *NE =
                  dyn_cast<CXXNewExpr>(Call.getOriginExpr())) {
     if (NE->isArray()) {
       arg1 = *NE->getArraySize();
     } else {
      //  return State;
      std::cout<<"return"<<std::endl;
      return;
     }
   } else
     llvm_unreachable("not a CallExpr or CXXNewExpr");

    // const Expr * arg1 = Call.getOriginExpr()->getArg(0);//getArgExpr(1);
    std::cout<<"-1"<<std::endl;
    // const Expr * arg1 = CE->getArg(0);//getArgExpr(1);
    if(arg1 == nullptr){
      std::cout<<"arg1=nullptr"<<std::endl;
    }
    std::cout<<"0"<<std::endl;
    SVal Argument = C.getSVal(arg1);









    std::cout<<"1"<<std::endl;
    // SVal Argument = Call.getObjectUnderConstruction();//lyf
    std::cout << "2" << std::endl;
    Optional<NonLoc> NL = Argument.getAs<NonLoc>();
    std::cout << "3" << std::endl;
    // return as the argument is not a concrete int
    // if(!NL || NL->getSubKind() != nonloc::ConcreteIntKind){
    //     return;
    // }
    std::cout << "4" << std::endl;
    // get the bit width of argument
    const auto Value = NL->castAs<nonloc::ConcreteInt>().getValue();
    std::cout << "5" << std::endl;
    unsigned Width =  Value.getBitWidth();
    ConstraintManager &CM = C.getConstraintManager();
    std::cout << "6" << std::endl;
    // two kinds of programstate
    ProgramStateRef stateNonPositive, statePositive;
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
      std::cout<<"argument in malloc should be positive"<<std::endl;
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
    }
    std::cout << "9" << std::endl;
  //} 




  



}
/*
State = ProcessZeroAllocCheck(Call, 0, State, Target);
// Performs a 0-sized allocations check.
 ProgramStateRef MallocChecker::ProcessZeroAllocCheck(
     const CallEvent &Call, const unsigned IndexOfSizeArg, ProgramStateRef State,
     Optional<SVal> RetVal) {
   if (!State)
     return nullptr;
 
   if (!RetVal)
     RetVal = Call.getReturnValue();
 
   const Expr *Arg = nullptr;
 
   if (const CallExpr *CE = dyn_cast<CallExpr>(Call.getOriginExpr())) {
     Arg = CE->getArg(IndexOfSizeArg);
   } else if (const CXXNewExpr *NE =
                  dyn_cast<CXXNewExpr>(Call.getOriginExpr())) {
     if (NE->isArray()) {
       Arg = *NE->getArraySize();
     } else {
       return State;
     }
   } else
     llvm_unreachable("not a CallExpr or CXXNewExpr");
 
   assert(Arg);
 
   auto DefArgVal =
       State->getSVal(Arg, Call.getLocationContext()).getAs<DefinedSVal>();
 
   if (!DefArgVal)
     return State;
 
   // Check if the allocation size is 0.
   ProgramStateRef TrueState, FalseState;
   SValBuilder &SvalBuilder = State->getStateManager().getSValBuilder();
   DefinedSVal Zero =
       SvalBuilder.makeZeroVal(Arg->getType()).castAs<DefinedSVal>();
 
   std::tie(TrueState, FalseState) =
       State->assume(SvalBuilder.evalEQ(State, *DefArgVal, Zero));
 
   if (TrueState && !FalseState) {
     SymbolRef Sym = RetVal->getAsLocSymbol();
     if (!Sym)
       return State;
 
     const RefState *RS = State->get<RegionState>(Sym);
     if (RS) {
       if (RS->isAllocated())
         return TrueState->set<RegionState>(Sym,
                                           RefState::getAllocatedOfSizeZero(RS));
       else
         return State;
     } else {
       // Case of zero-size realloc. Historically 'realloc(ptr, 0)' is treated as
       // 'free(ptr)' and the returned value from 'realloc(ptr, 0)' is not
       // tracked. Add zero-reallocated Sym to the state to catch references
       // to zero-allocated memory.
       return TrueState->add<ReallocSizeZeroSymbols>(Sym);
     }
   }
 
   // Assume the value is non-zero going forward.
   assert(FalseState);
   return FalseState;
 }*/