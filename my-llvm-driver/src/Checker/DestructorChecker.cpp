#include "Checker/DestructorChecker.h"
using namespace clang;
using namespace ento;

bool inDestructor;

void DestructorChecker::checkBeginFunction(CheckerContext &C) const {
    if (!C.inTopFrame())
        return;
    const auto *LCtx = C.getLocationContext();
    const auto *MD = dyn_cast<CXXDestructorDecl>(LCtx->getDecl());
    if (!MD)
        return;
    inDestructor = true;
}

void DestructorChecker::checkEndFunction(const ReturnStmt *RS, CheckerContext &C) const {
    inDestructor = false;
}

void DestructorChecker::checkPreStmt(const CallExpr *CE,CheckerContext &C) const {
    if(inDestructor && CE->getDirectCallee()->getNameAsString() == "exit") {
        if (!BT) {
            BT.reset(new BugType(this,
            "exit() shouldn't be called by destructor","DestructorChecker"));
        }
        ExplodedNode *N = C.generateErrorNode();
        auto Report = std::make_unique<PathSensitiveBugReport>(*BT,
                        "exit() shouldn't be called by destructor",N);
        // report explicitly
        C.emitReport(std::move(Report));
    }
}