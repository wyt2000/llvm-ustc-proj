#ifndef __MALLOC_OVERFLOW_H__
#define __MALLOC_OVERFLOW_H__

// MallocOverflowSecurityChecker.cpp - Check for malloc overflows -*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This checker detects a common memory allocation security flaw.
// Suppose 'unsigned int n' comes from an untrusted source. If the
// code looks like 'malloc (n * 4)', and an attacker can make 'n' be
// say MAX_UINT/4+2, then instead of allocating the correct 'n' 4-byte
// elements, this will actually allocate only two because of overflow.
// Then when the rest of the program attempts to store values past the
// second element, these values will actually overwrite other items in
// the heap, probably allowing the attacker to execute arbitrary code.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/SmallVector.h"
#include <utility>

using llvm::APSInt;

namespace clang
{
namespace ento
{

struct MallocOverflowCheck {
  const BinaryOperator *mulop;
  const Expr *variable;
  APSInt maxVal;

  MallocOverflowCheck(const BinaryOperator *m, const Expr *v, APSInt val)
      : mulop(m), variable(v), maxVal(std::move(val)) {}

};

class MallocOverflowSecurityChecker : public Checker<check::ASTCodeBody> {

    public:
    void checkASTCodeBody(const Decl *D, AnalysisManager &mgr,
                        BugReporter &BR) const;

    void CheckMallocArgument(
        SmallVectorImpl<MallocOverflowCheck> &PossibleMallocOverflows,
        const Expr *TheArgument, ASTContext &Context) const;

    void OutputPossibleOverflows(
        SmallVectorImpl<MallocOverflowCheck> &PossibleMallocOverflows,
        const Decl *D, BugReporter &BR, AnalysisManager &mgr) const;

};

//} // end anonymous namespace
//}

// Return true for computations which evaluate to zero: e.g., mult by 0.
static inline bool EvaluatesToZero(APSInt &Val, BinaryOperatorKind op) {
  return (op == BO_Mul) && (Val == 0);
}

// A worker class for OutputPossibleOverflows.
class CheckOverflowOps : public EvaluatedExprVisitor<CheckOverflowOps> {
    public:
        typedef SmallVectorImpl<MallocOverflowCheck> theVecType;

    private:
        theVecType &toScanFor;
        ASTContext &Context;

    bool isIntZeroExpr(const Expr *E) const;

    static const Decl *getDecl(const DeclRefExpr *DR) { return DR->getDecl(); }
    static const Decl *getDecl(const MemberExpr *ME) {
      return ME->getMemberDecl();
    }

    template <typename T1>
    void Erase(const T1 *DR,
               llvm::function_ref<bool(const MallocOverflowCheck &)> Pred);

    void CheckExpr(const Expr *E_p);

    // Check if the argument to malloc is assigned a value
    // which cannot cause an overflow.
    // e.g., malloc (mul * x) and,
    // case 1: mul = <constant value>
    // case 2: mul = a/b, where b > x
    void CheckAssignmentExpr(BinaryOperator *AssignEx);
  public:
    void VisitBinaryOperator(BinaryOperator *E);

    /* We specifically ignore loop conditions, because they're typically
     not error checks.  */
    void VisitWhileStmt(WhileStmt *S) {
      return this->Visit(S->getBody());
    }
    void VisitForStmt(ForStmt *S) {
      return this->Visit(S->getBody());
    }
    void VisitDoStmt(DoStmt *S) {
      return this->Visit(S->getBody());
    }

    CheckOverflowOps(theVecType &v, ASTContext &ctx)
    : EvaluatedExprVisitor<CheckOverflowOps>(ctx),
      toScanFor(v), Context(ctx)
    { }
};

// end of namespace
}
}

#endif
