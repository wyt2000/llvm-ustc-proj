#include "Checker/DumbChecker.h"
using namespace clang;
using namespace ento;
using llvm::APSInt;

// check whether the argument of new is not positive.
// if so, report the error
void DumbChecker::checkPreCall(const CallEvent &Call, CheckerContext &C) const {
    
}