#include "Checker/MallocOverflowSecurityChecker.h"
using namespace clang;
using namespace ento;
using llvm::APSInt;

void MallocOverflowSecurityChecker::checkASTCodeBody(const Decl *D,
                                             AnalysisManager &mgr,
                                             BugReporter &BR) const {                                         
  CFG *cfg = mgr.getCFG(D);
  if (!cfg)
    return;

  // A list of variables referenced in possibly overflowing malloc operands.
  SmallVector<MallocOverflowCheck, 2> PossibleMallocOverflows;

  for (CFG::iterator it = cfg->begin(), ei = cfg->end(); it != ei; ++it) {
    CFGBlock *block = *it;
    for (CFGBlock::iterator bi = block->begin(), be = block->end();
         bi != be; ++bi) {
      if (Optional<CFGStmt> CS = bi->getAs<CFGStmt>()) {
        if (const CallExpr *TheCall = dyn_cast<CallExpr>(CS->getStmt())) {
          // Get the callee.
          const FunctionDecl *FD = TheCall->getDirectCallee();

          if (!FD)
            continue;

          // Get the name of the callee. If it's a builtin, strip off the prefix.
          IdentifierInfo *FnInfo = FD->getIdentifier();
          if (!FnInfo)
            continue;

          if (FnInfo->isStr ("malloc") || FnInfo->isStr ("_MALLOC")) {
            if (TheCall->getNumArgs() == 1)
              CheckMallocArgument(PossibleMallocOverflows, TheCall->getArg(0),
                                  mgr.getASTContext());
          }
        }
      }
    }
  }

  OutputPossibleOverflows(PossibleMallocOverflows, D, BR, mgr);
}

void MallocOverflowSecurityChecker::CheckMallocArgument(
  SmallVectorImpl<MallocOverflowCheck> &PossibleMallocOverflows,
  const Expr *TheArgument,
  ASTContext &Context) const {

  /* Look for a linear combination with a single variable, and at least
   one multiplication.
   Reject anything that applies to the variable: an explicit cast,
   conditional expression, an operation that could reduce the range
   of the result, or anything too complicated :-).  */
  const Expr *e = TheArgument;
  const BinaryOperator * mulop = nullptr;
  APSInt maxVal;

  for (;;) {
    maxVal = 0;
    e = e->IgnoreParenImpCasts();
    if (const BinaryOperator *binop = dyn_cast<BinaryOperator>(e)) {
      BinaryOperatorKind opc = binop->getOpcode();
      // TODO: ignore multiplications by 1, reject if multiplied by 0.
      if (mulop == nullptr && opc == BO_Mul)
        mulop = binop;
      if (opc != BO_Mul && opc != BO_Add && opc != BO_Sub && opc != BO_Shl)
        return;

      const Expr *lhs = binop->getLHS();
      const Expr *rhs = binop->getRHS();
      if (rhs->isEvaluatable(Context)) {
        e = lhs;
        maxVal = rhs->EvaluateKnownConstInt(Context);
        if (EvaluatesToZero(maxVal, opc))
          return;
      } else if ((opc == BO_Add || opc == BO_Mul) &&
                 lhs->isEvaluatable(Context)) {
        maxVal = lhs->EvaluateKnownConstInt(Context);
        if (EvaluatesToZero(maxVal, opc))
          return;
        e = rhs;
      } else
        return;
    }
    else if (isa<DeclRefExpr>(e) || isa<MemberExpr>(e))
      break;
    else
      return;
  }

  if (mulop == nullptr)
    return;

  //  We've found the right structure of malloc argument, now save
  // the data so when the body of the function is completely available
  // we can check for comparisons.

  // TODO: Could push this into the innermost scope where 'e' is
  // defined, rather than the whole function.
  PossibleMallocOverflows.push_back(MallocOverflowCheck(mulop, e, maxVal));
}

// OutputPossibleOverflows - We've found a possible overflow earlier,
// now check whether Body might contain a comparison which might be
// preventing the overflow.
// This doesn't do flow analysis, range analysis, or points-to analysis; it's
// just a dumb "is there a comparison" scan.  The aim here is to
// detect the most blatent cases of overflow and educate the
// programmer.
void MallocOverflowSecurityChecker::OutputPossibleOverflows(
  SmallVectorImpl<MallocOverflowCheck> &PossibleMallocOverflows,
  const Decl *D, BugReporter &BR, AnalysisManager &mgr) const {  
  // By far the most common case: nothing to check.
  if (PossibleMallocOverflows.empty())
    return;

  // Delete any possible overflows which have a comparison.
  CheckOverflowOps c(PossibleMallocOverflows, BR.getContext());
  c.Visit(mgr.getAnalysisDeclContext(D)->getBody());

  // Output warnings for all overflows that are left.
  for (CheckOverflowOps::theVecType::iterator
       i = PossibleMallocOverflows.begin(),
       e = PossibleMallocOverflows.end();
       i != e;
       ++i) {
    BR.EmitBasicReport(
        D, this, "malloc() size overflow", categories::UnixAPI,
        "the computation of the size of the memory allocation may overflow",
        PathDiagnosticLocation::createOperatorLoc(i->mulop,
                                                  BR.getSourceManager()),
        i->mulop->getSourceRange());
  }
}


bool CheckOverflowOps::isIntZeroExpr(const Expr *E) const {
  if (!E->getType()->isIntegralOrEnumerationType())
    return false;
  Expr::EvalResult Result;
  if (E->EvaluateAsInt(Result, Context))
    return Result.Val.getInt() == 0;
  return false;
}

template <typename T1>
void CheckOverflowOps::Erase(const T1 *DR,
        llvm::function_ref<bool(const MallocOverflowCheck &)> Pred){
  auto P = [DR, Pred](const MallocOverflowCheck &Check) {
  if (const auto *CheckDR = dyn_cast<T1>(Check.variable))
    return getDecl(CheckDR) == getDecl(DR) && Pred(Check);
  return false;
  };
  toScanFor.erase(std::remove_if(toScanFor.begin(), toScanFor.end(), P),
                      toScanFor.end());
}

void CheckOverflowOps::CheckExpr(const Expr *E_p) {
      auto PredTrue = [](const MallocOverflowCheck &) { return true; };
      const Expr *E = E_p->IgnoreParenImpCasts();
      if (const DeclRefExpr *DR = dyn_cast<DeclRefExpr>(E))
        Erase<DeclRefExpr>(DR, PredTrue);
      else if (const auto *ME = dyn_cast<MemberExpr>(E)) {
        Erase<MemberExpr>(ME, PredTrue);
      }
}

void CheckOverflowOps::CheckAssignmentExpr(BinaryOperator *AssignEx){
      bool assignKnown = false;
      bool numeratorKnown = false, denomKnown = false;
      APSInt denomVal;
      denomVal = 0;

      // Erase if the multiplicand was assigned a constant value.
      const Expr *rhs = AssignEx->getRHS();
      if (rhs->isEvaluatable(Context))
        assignKnown = true;

      // Discard the report if the multiplicand was assigned a value,
      // that can never overflow after multiplication. e.g., the assignment
      // is a division operator and the denominator is > other multiplicand.
      const Expr *rhse = rhs->IgnoreParenImpCasts();
      if (const BinaryOperator *BOp = dyn_cast<BinaryOperator>(rhse)) {
        if (BOp->getOpcode() == BO_Div) {
          const Expr *denom = BOp->getRHS()->IgnoreParenImpCasts();
          Expr::EvalResult Result;
          if (denom->EvaluateAsInt(Result, Context)) {
            denomVal = Result.Val.getInt();
            denomKnown = true;
          }
          const Expr *numerator = BOp->getLHS()->IgnoreParenImpCasts();
          if (numerator->isEvaluatable(Context))
            numeratorKnown = true;
        }
      }
      if (!assignKnown && !denomKnown)
        return;
      auto denomExtVal = denomVal.getExtValue();

      // Ignore negative denominator.
      if (denomExtVal < 0)
        return;

      const Expr *lhs = AssignEx->getLHS();
      const Expr *E = lhs->IgnoreParenImpCasts();

      auto pred = [assignKnown, numeratorKnown,
                   denomExtVal](const MallocOverflowCheck &Check) {
        return assignKnown ||
               (numeratorKnown && (denomExtVal >= Check.maxVal.getExtValue()));
      };

      if (const DeclRefExpr *DR = dyn_cast<DeclRefExpr>(E))
        Erase<DeclRefExpr>(DR, pred);
      else if (const auto *ME = dyn_cast<MemberExpr>(E))
        Erase<MemberExpr>(ME, pred);
}

void CheckOverflowOps::VisitBinaryOperator(BinaryOperator *E) {
      if (E->isComparisonOp()) {
        const Expr * lhs = E->getLHS();
        const Expr * rhs = E->getRHS();
        // Ignore comparisons against zero, since they generally don't
        // protect against an overflow.
        if (!isIntZeroExpr(lhs) && !isIntZeroExpr(rhs)) {
          CheckExpr(lhs);
          CheckExpr(rhs);
        }
      }
      if (E->isAssignmentOp())
        CheckAssignmentExpr(E);
      EvaluatedExprVisitor<CheckOverflowOps>::VisitBinaryOperator(E);
}
