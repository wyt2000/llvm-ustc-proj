#ifndef __DESTRUCTOR_STATIC_CHECKER_H__
#define __DESTRUCTOR_STATIC_CHECKER_H__

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include <utility>

using llvm::APSInt;

namespace clang
{
namespace ento
{

class DestructorStaChecker : public Checker<   check::BeginFunction,
                                               check::EndFunction,
                                               check::PreStmt<DeclStmt> > {
    bool CheckDestructor(CheckerContext &C) const;
    public:
        mutable std::unique_ptr<BugType> BT;
        void checkBeginFunction(CheckerContext &C) const;
        void checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const;
        void checkPreStmt(const DeclStmt *DS, CheckerContext &C) const;
};

// end of namespace
}
}

#endif
