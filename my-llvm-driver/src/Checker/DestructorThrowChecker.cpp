#include "Checker/DestructorThrowChecker.h"
#include<iostream>
using namespace clang;
using namespace ento;

bool inDestructor3;

bool DestructorThrowChecker::CheckDestructor(CheckerContext &C) const {
    if (!C.inTopFrame())
        return false;
    const auto *LCtx = C.getLocationContext();
    const auto *MD = dyn_cast<CXXDestructorDecl>(LCtx->getDecl());
    if (!MD)
        return false;
    return true;
}

void DestructorThrowChecker::checkBeginFunction(CheckerContext &C) const {
    if (CheckDestructor(C)) {
        inDestructor3 = true;
    }
    else {
        inDestructor3 = false;
    }
}

void DestructorThrowChecker::checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const {
    if (CheckDestructor(C)) {
        inDestructor3 = false;
    }
}

void DestructorThrowChecker::checkPreStmt(const CallExpr *CE, CheckerContext &C) const {
    if(inDestructor3) {
        const FunctionDecl* D = CE->getDirectCallee();
        if (!D) {
            return;
        }
        const Stmt* S = D->getDefinition()->getBody();
        for (const auto *Child : S->children()) {
            const CXXThrowExpr* TE = dyn_cast<CXXThrowExpr>(Child);
            if (!TE) {
                return;
            }
            if (!BT) {
                BT.reset(new BugType(this,
                "throw shouldn't be declared by destructor","DestructorThrowChecker"));
            }
            ExplodedNode *N = C.generateErrorNode();
            auto Report = std::make_unique<PathSensitiveBugReport>(*BT,
                            "throw shouldn't be declared by destructor",N);
            // report explicitly
            C.emitReport(std::move(Report));
        }
        inDestructor3 = false;
    }
}