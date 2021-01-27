#include "Checker/LeakEvalOrderChecker.h"
#include <iostream>
using namespace clang;
using namespace ento;

REGISTER_MAP_WITH_PROGRAMSTATE(MemLeakMap, const CallExpr*, LeakEvalOrderChecker::ReturnAndMalloc)

bool LeakEvalOrderChecker::getNoReturn(const CallExpr *CE,CheckerContext &C) const {
    const FunctionDecl* D = CE->getDirectCallee();
    if (!D) {
        return false;
    }
    if (D->isNoReturn()) {
        return true;
    }
    if (!D->getDefinition()) {
        return false;
    }
    const Stmt* S = D->getDefinition()->getBody();
    for (const auto *Child : S->children()) {
        const CXXThrowExpr* TE = dyn_cast<CXXThrowExpr>(Child);
        if (TE) {
            return true;
        }
        const CallExpr* CC = dyn_cast<CallExpr>(Child);
        if (CC) {
            D = CC->getDirectCallee();
            if (D && D->isNoReturn()) {
                return true;
            }
        }
    }
    return false;
}

bool LeakEvalOrderChecker::getMallocOrNew(const CallExpr *CE,CheckerContext &C) const {
    if(!CE->getNumArgs()) {
        return false;
    }
    if (CE->getDirectCallee()->getNameAsString() == "malloc") {
        return true;
    }
    const CXXNewExpr *NE = dyn_cast<CXXNewExpr>(CE->getArg(0));
    if (NE) {
        return true;
    }
    return false;
}

void LeakEvalOrderChecker::checkPreStmt(const CallExpr *CE,CheckerContext &C) const {
    unsigned num = CE->getNumArgs();
    ProgramStateRef State = C.getState();
    bool NoReturn = getNoReturn(CE, C);
    bool MallocOrNew = getMallocOrNew(CE, C);
    for (unsigned i = 0; i < num; i++) {
        const CallExpr* CallArg = dyn_cast<CallExpr>(CE->getArg(i));
        if (!CallArg) {
            continue;
        }
        const ReturnAndMalloc* RAM = State->get<MemLeakMap>(CallArg);
        if (RAM) {
            NoReturn |= RAM->getNoReturn();
            MallocOrNew |= RAM->getMallocOrNew();
        }
    }
    if (NoReturn && MallocOrNew) {
        ReportBug(C);
        return;
    }
    State = State->set<MemLeakMap>(CE, ReturnAndMalloc(NoReturn, MallocOrNew));
    C.addTransition(State);
}

void LeakEvalOrderChecker::ReportBug(CheckerContext &C) const {
    if (!BT) {
         BT.reset(new BugType(this,
        "There may be memory leaks","LeakEvalOrderChecker"));
    }
    ExplodedNode *N = C.generateErrorNode();
    auto Report = std::make_unique<PathSensitiveBugReport>(*BT,
                    "There may be memory leaks",N);
    // report explicitly
    C.emitReport(std::move(Report));
}