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
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
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

class MallocOverflowSecurityChecker : public Checker<check::PreStmt<CallExpr>> {

    public:
    mutable std::unique_ptr<BugType> BT;
    // check the argument before call function statement.(on AST)
    void checkPreStmt(const CallExpr *CE,CheckerContext &C) const;

};

// end of namespace
}
}

#endif
