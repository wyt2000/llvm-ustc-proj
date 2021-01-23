#ifndef __NEW_ZERO_H__
#define __NEW_ZERO_H__

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/SmallVector.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include <utility>

using llvm::APSInt;

namespace clang
{
namespace ento
{

struct NewZeroCheck {
  const BinaryOperator *mulop;
  const Expr *variable;
  APSInt maxVal;

  NewZeroCheck(const BinaryOperator *m, const Expr *v, APSInt val)
      : mulop(m), variable(v), maxVal(std::move(val)) {}

};

class NewZeroChecker : public Checker<check::NewAllocator> {

    public:
    mutable std::unique_ptr<BugType> BT;
    // check the argument before call function statement.(on AST)
    void checkNewAllocator(const CXXAllocatorCall &Call,
                                  CheckerContext &C) const;

};

// end of namespace
}
}

#endif