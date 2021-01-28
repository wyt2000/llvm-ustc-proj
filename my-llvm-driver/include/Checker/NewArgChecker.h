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

class NewArgChecker : public Checker<check::NewAllocator> {
public:
    mutable std::unique_ptr<BugType> BT;
    // check the argument of new
    void checkNewAllocator(const CXXAllocatorCall &Call,
                                  CheckerContext &C) const;
};

// end of namespace
}
}

#endif