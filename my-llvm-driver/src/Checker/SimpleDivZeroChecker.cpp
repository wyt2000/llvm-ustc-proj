#include "Checker/SimpleDivZeroChecker.h"
#include <iostream>
#include <limits.h>

using namespace clang;
using namespace ento;

static const Expr *getDenomExpr(const ExplodedNode *N) {
  const Stmt *S = N->getLocationAs<PreStmt>()->getStmt();
  if (const auto *BE = dyn_cast<BinaryOperator>(S))
    return BE->getRHS();
  return nullptr;
}

void SimpleDivChecker::reportBug(
    const char *Msg, ProgramStateRef StateZero, CheckerContext &C,
    std::unique_ptr<BugReporterVisitor> Visitor) const {
  if (ExplodedNode *N = C.generateErrorNode(StateZero)) {
    if (!BT)
      BT.reset(new BuiltinBug(this, "Division by negative"));

    auto R = std::make_unique<PathSensitiveBugReport>(*BT, Msg, N);
    R->addVisitor(std::move(Visitor));
    bugreporter::trackExpressionValue(N, getDenomExpr(N), *R);
    C.emitReport(std::move(R));
  }
}

void SimpleDivChecker::checkPreStmt(const BinaryOperator *B,
                                  CheckerContext &C) const {
  BinaryOperator::Opcode Op = B->getOpcode();
  if (Op != BO_Div &&
      Op != BO_Rem &&
      Op != BO_DivAssign &&
      Op != BO_RemAssign)
    return;

  if (!B->getRHS()->getType()->isScalarType())
    return;

  SVal Denom = C.getSVal(B->getRHS());
  Optional<NonLoc> NL = Denom.getAs<NonLoc>();
  if(!NL || NL->getSubKind() != nonloc::ConcreteIntKind)
    return;
  
  const auto &Value = NL->castAs<nonloc::ConcreteInt>().getValue();
  if(!Value.isSigned()) {
    return;
  }
  unsigned Width =  Value.getBitWidth();

  ConstraintManager &CM = C.getConstraintManager();
  ProgramStateRef stateNegaive, stateNonNegative;
  std::tie(stateNegaive, stateNonNegative) = CM.assumeInclusiveRangeDual(C.getState(), 
                                                                        *NL, 
                                                                        llvm::APSInt(llvm::APInt(Width, 0), false),
                                                                        llvm::APSInt::getMaxValue(Width, false)
                                                                        );
  
  if (!stateNegaive) {
    assert(stateNonNegative);
    reportBug("Division by negative", stateNonNegative, C);
    return;
  }

  C.addTransition(stateNegaive);
}

/*  TODO:
  * 1. global variable
  * 2. float
  * 3. non-ConcreteIntKind ?
*/