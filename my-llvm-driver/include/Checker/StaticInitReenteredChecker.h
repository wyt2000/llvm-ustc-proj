#ifndef __STATIC_INIT_REENTERED_H__
#define __STATIC_INIT_REENTERED_H__

// static_init_reentered.h - Check for recursive init of static variables -*- C++ -*-=//
//
//
//===----------------------------------------------------------------------===//
#include <vector>
#include "Checker/InterCheckerAPI.h"
#include "clang/Basic/CharInfo.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/DynamicSize.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/raw_ostream.h"


namespace clang
{
namespace ento
{

// records the state of every function
struct FunctionState{
    std::string name;
    std::vector<int> callees;
    int parent = -1;
};

// check recursive declaration of static variables
class StaticInitReenteredChecker : public Checker<
                            check::BeginFunction,
                            check::EndFunction,
                            check::PreStmt<DeclStmt> > {
    public:
        mutable std::unique_ptr<BugType> BT;
        void checkPreStmt(const DeclStmt *DS, CheckerContext &C) const;
        void checkBeginFunction(CheckerContext &C) const;
        void checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const;
        void reportBug(const char *Msg,CheckerContext &C) const;
};

// end of namespace
}
}

#endif
