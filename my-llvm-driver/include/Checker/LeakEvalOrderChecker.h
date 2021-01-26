#ifndef __LEAK_EVAL_ORDER_CHECKER_H__
#define __LEAK_EVAL_ORDER_CHECKER_H__

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

class LeakEvalOrderChecker : public Checker<check::PreStmt<CallExpr> > {
    public:
        struct ReturnAndMalloc {
            private:
                bool NoReturn;
                bool MallocOrNew;
            public:
                ReturnAndMalloc(bool N, bool M) : NoReturn(N), MallocOrNew(M) { }
                bool getNoReturn() const {return NoReturn;}
                bool getMallocOrNew() const {return MallocOrNew;}
                bool operator == (const ReturnAndMalloc &X) const {
                    return NoReturn == X.NoReturn && MallocOrNew == X.MallocOrNew;
                }
                void Profile(llvm::FoldingSetNodeID &ID) const {
                    ID.AddInteger(1);
                }
        };
        mutable std::unique_ptr<BugType> BT;
        void ReportBug(CheckerContext &C) const;
        void checkPreStmt(const CallExpr *CE,CheckerContext &C) const;
        bool getNoReturn(const CallExpr *CE,CheckerContext &C) const;
        bool getMallocOrNew(const CallExpr *CE,CheckerContext &C) const;
};

// end of namespace
}
}

#endif