#include "Checker/DestructorStaChecker.h"
#include<iostream>
using namespace clang;
using namespace ento;

bool inDestructor2;

bool DestructorStaChecker::CheckDestructor(CheckerContext &C) const {
    if (!C.inTopFrame())
        return false;
    const auto *LCtx = C.getLocationContext();
    const auto *MD = dyn_cast<CXXDestructorDecl>(LCtx->getDecl());
    if (!MD)
        return false;
    return true;
}

void DestructorStaChecker::checkBeginFunction(CheckerContext &C) const {
    if (CheckDestructor(C)) {
        inDestructor2 = true;
    }
}

void DestructorStaChecker::checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const {
    if (CheckDestructor(C)) {
        inDestructor2 = false;
    }
}

void DestructorStaChecker::checkPreStmt(const DeclStmt *DS, CheckerContext &C) const {
    if(inDestructor2) {
        for(const auto *I : DS->decls()) {
            const VarDecl *VD = dyn_cast<VarDecl>(I);
            if(!VD)
                continue;
            if(VD->isStaticLocal()) {
                if (!BT) {
                    BT.reset(new BugType(this,
                    "static variable shouldn't be declared by destructor","DestructorStaChecker"));
                }
                ExplodedNode *N = C.generateErrorNode();
                auto Report = std::make_unique<PathSensitiveBugReport>(*BT,
                                "static variable shouldn't be declared by destructor",N);
                // report explicitly
                C.emitReport(std::move(Report));
            }
        }
    }
}