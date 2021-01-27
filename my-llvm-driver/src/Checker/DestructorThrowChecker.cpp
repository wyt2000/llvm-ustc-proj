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
}

void DestructorThrowChecker::checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const {
    if (CheckDestructor(C)) {
        inDestructor3 = false;
    }
}

void DestructorThrowChecker::checkPreStmt(const CXXThrowExpr *TE, CheckerContext &C) const {
    if(inDestructor3) {
        /*for(const auto *I : DS->decls()) {
            const VarDecl *VD = dyn_cast<VarDecl>(I);
            if(!VD)
                continue;
            if(VD->isStaticLocal()) {*/
        std::cout << "in destructor" << std::endl;
        /*const Expr *Exp = dyn_cast<Expr>(VS);
        if(!Exp) {
            std::cout << "not expr" << std::endl;
            return;
        }
        const CXXThrowExpr *Throw = dyn_cast<CXXThrowExpr>(Exp);
        if(!Throw) {
            std::cout << "not CXXThrowExpr" << std::endl;
            return;
        }*/
        std::cout << "throw in destructor" << std::endl;
        if (!BT) {
            BT.reset(new BugType(this,
            "throw shouldn't be declared by destructor","DestructorStaChecker"));
        }
        ExplodedNode *N = C.generateErrorNode();
        auto Report = std::make_unique<PathSensitiveBugReport>(*BT,
                        "static variable shouldn't be declared by destructor",N);
        // report explicitly
        C.emitReport(std::move(Report));
            //}
        //}
    }
}