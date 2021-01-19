#ifndef __SIMPLE_STREAM_CHECKER_H__
#define __SIMPLE_STREAM_CHECKER_H__

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <utility>

namespace clang
{
namespace ento
{

typedef SmallVector<SymbolRef, 2> SymbolVector;

struct StreamState {
private:
  enum Kind { Opened, Closed } K;
  StreamState(Kind InK) : K(InK) { }

public:
  bool isOpened() const { return K == Opened; }
  bool isClosed() const { return K == Closed; }

  static StreamState getOpened() { return StreamState(Opened); }
  static StreamState getClosed() { return StreamState(Closed); }

  bool operator==(const StreamState &X) const {
    return K == X.K;
  }
  void Profile(llvm::FoldingSetNodeID &ID) const {
    ID.AddInteger(K);
  }
};

class SimpleStreamChecker : public Checker<check::PostCall,
                                           check::PreCall,
                                           check::DeadSymbols,
                                           check::PointerEscape> {
  CallDescription OpenFn, CloseFn;

  std::unique_ptr<BugType> DoubleCloseBugType;
  std::unique_ptr<BugType> LeakBugType;

  void reportDoubleClose(SymbolRef FileDescSym,
                         const CallEvent &Call,
                         CheckerContext &C) const;

  void reportLeaks(ArrayRef<SymbolRef> LeakedStreams, CheckerContext &C,
                   ExplodedNode *ErrNode) const;

  bool guaranteedNotToCloseFile(const CallEvent &Call) const;

public:
  SimpleStreamChecker();

  /// Process fopen.
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
  /// Process fclose.
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  void checkDeadSymbols(SymbolReaper &SymReaper, CheckerContext &C) const;

  /// Stop tracking addresses which escape.
  ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                    const InvalidatedSymbols &Escaped,
                                    const CallEvent *Call,
                                    PointerEscapeKind Kind) const;
};

}
}

#endif