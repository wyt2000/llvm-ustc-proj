#ifndef __CSTRING_CHECKER_H__
#define __CSTRING_CHECKER_H__

//= CStringChecker.cpp - Checks calls to C string functions --------*- C++ -*-//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This defines CStringChecker, which is an assortment of checks on calls
// to functions in <string.h>.
//
//===----------------------------------------------------------------------===//

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

namespace clang {
namespace ento {
    struct AnyArgExpr {
    // FIXME: Remove constructor in C++17 to turn it into an aggregate.
    AnyArgExpr(const Expr *Expression, unsigned ArgumentIndex)
        : Expression{Expression}, ArgumentIndex{ArgumentIndex} {}
    const Expr *Expression;
    unsigned ArgumentIndex;
    };

    struct SourceArgExpr : AnyArgExpr {
    using AnyArgExpr::AnyArgExpr; // FIXME: Remove using in C++17.
    };

    struct DestinationArgExpr : AnyArgExpr {
    using AnyArgExpr::AnyArgExpr; // FIXME: Same.
    };

    struct SizeArgExpr : AnyArgExpr {
    using AnyArgExpr::AnyArgExpr; // FIXME: Same.
    };

    using ErrorMessage = SmallString<128>;
    enum class AccessKind { write, read };

    static ErrorMessage createOutOfBoundErrorMsg(StringRef FunctionDescription,
                                                AccessKind Access) {
    ErrorMessage Message;
    llvm::raw_svector_ostream Os(Message);

    // Function classification like: Memory copy function
    Os << toUppercase(FunctionDescription.front())
        << &FunctionDescription.data()[1];

    if (Access == AccessKind::write) {
        Os << " overflows the destination buffer";
    } else { // read access
        Os << " accesses out-of-bound array element";
    }

    return Message;
    }

    enum class ConcatFnKind { none = 0, strcat = 1, strlcat = 2 };
    class CStringChecker : public Checker< eval::Call,
                                            check::PreStmt<DeclStmt>,
                                            check::LiveSymbols,
                                            check::DeadSymbols,
                                            check::RegionChanges
                                            > {
    mutable std::unique_ptr<BugType> BT_Null, BT_Bounds, BT_Overlap,
        BT_NotCString, BT_AdditionOverflow;

    mutable const char *CurrentFunctionDescription;

    public:
    /// The filter is used to filter out the diagnostics which are not enabled by
    /// the user.
    struct CStringChecksFilter {
        DefaultBool CheckCStringNullArg;
        DefaultBool CheckCStringOutOfBounds;
        DefaultBool CheckCStringBufferOverlap;
        DefaultBool CheckCStringNotNullTerm;

        CheckerNameRef CheckNameCStringNullArg;
        CheckerNameRef CheckNameCStringOutOfBounds;
        CheckerNameRef CheckNameCStringBufferOverlap;
        CheckerNameRef CheckNameCStringNotNullTerm;
    };

    CStringChecksFilter Filter;

    static void *getTag() { static int tag; return &tag; }

    bool evalCall(const CallEvent &Call, CheckerContext &C) const;
    void checkPreStmt(const DeclStmt *DS, CheckerContext &C) const;
    void checkLiveSymbols(ProgramStateRef state, SymbolReaper &SR) const;
    void checkDeadSymbols(SymbolReaper &SR, CheckerContext &C) const;

    ProgramStateRef
        checkRegionChanges(ProgramStateRef state,
                        const InvalidatedSymbols *,
                        ArrayRef<const MemRegion *> ExplicitRegions,
                        ArrayRef<const MemRegion *> Regions,
                        const LocationContext *LCtx,
                        const CallEvent *Call) const;

    typedef void (CStringChecker::*FnCheck)(CheckerContext &,
                                            const CallExpr *) const;
    CallDescriptionMap<FnCheck> Callbacks = {
        {{CDF_MaybeBuiltin, "memcpy", 3}, &CStringChecker::evalMemcpy},
        {{CDF_MaybeBuiltin, "mempcpy", 3}, &CStringChecker::evalMempcpy},
        {{CDF_MaybeBuiltin, "memcmp", 3}, &CStringChecker::evalMemcmp},
        {{CDF_MaybeBuiltin, "memmove", 3}, &CStringChecker::evalMemmove},
        {{CDF_MaybeBuiltin, "memset", 3}, &CStringChecker::evalMemset},
        {{CDF_MaybeBuiltin, "explicit_memset", 3}, &CStringChecker::evalMemset},
        {{CDF_MaybeBuiltin, "strcpy", 2}, &CStringChecker::evalStrcpy},
        {{CDF_MaybeBuiltin, "strncpy", 3}, &CStringChecker::evalStrncpy},
        {{CDF_MaybeBuiltin, "stpcpy", 2}, &CStringChecker::evalStpcpy},
        {{CDF_MaybeBuiltin, "strlcpy", 3}, &CStringChecker::evalStrlcpy},
        {{CDF_MaybeBuiltin, "strcat", 2}, &CStringChecker::evalStrcat},
        {{CDF_MaybeBuiltin, "strncat", 3}, &CStringChecker::evalStrncat},
        {{CDF_MaybeBuiltin, "strlcat", 3}, &CStringChecker::evalStrlcat},
        {{CDF_MaybeBuiltin, "strlen", 1}, &CStringChecker::evalstrLength},
        {{CDF_MaybeBuiltin, "strnlen", 2}, &CStringChecker::evalstrnLength},
        {{CDF_MaybeBuiltin, "strcmp", 2}, &CStringChecker::evalStrcmp},
        {{CDF_MaybeBuiltin, "strncmp", 3}, &CStringChecker::evalStrncmp},
        {{CDF_MaybeBuiltin, "strcasecmp", 2}, &CStringChecker::evalStrcasecmp},
        {{CDF_MaybeBuiltin, "strncasecmp", 3}, &CStringChecker::evalStrncasecmp},
        {{CDF_MaybeBuiltin, "strsep", 2}, &CStringChecker::evalStrsep},
        {{CDF_MaybeBuiltin, "bcopy", 3}, &CStringChecker::evalBcopy},
        {{CDF_MaybeBuiltin, "bcmp", 3}, &CStringChecker::evalMemcmp},
        {{CDF_MaybeBuiltin, "bzero", 2}, &CStringChecker::evalBzero},
        {{CDF_MaybeBuiltin, "explicit_bzero", 2}, &CStringChecker::evalBzero},
    };

    // These require a bit of special handling.
    CallDescription StdCopy{{"std", "copy"}, 3},
        StdCopyBackward{{"std", "copy_backward"}, 3};

    FnCheck identifyCall(const CallEvent &Call, CheckerContext &C) const;
    void evalMemcpy(CheckerContext &C, const CallExpr *CE) const;
    void evalMempcpy(CheckerContext &C, const CallExpr *CE) const;
    void evalMemmove(CheckerContext &C, const CallExpr *CE) const;
    void evalBcopy(CheckerContext &C, const CallExpr *CE) const;
    void evalCopyCommon(CheckerContext &C, const CallExpr *CE,
                        ProgramStateRef state, SizeArgExpr Size,
                        DestinationArgExpr Dest, SourceArgExpr Source,
                        bool Restricted, bool IsMempcpy) const;

    void evalMemcmp(CheckerContext &C, const CallExpr *CE) const;

    void evalstrLength(CheckerContext &C, const CallExpr *CE) const;
    void evalstrnLength(CheckerContext &C, const CallExpr *CE) const;
    void evalstrLengthCommon(CheckerContext &C,
                            const CallExpr *CE,
                            bool IsStrnlen = false) const;

    void evalStrcpy(CheckerContext &C, const CallExpr *CE) const;
    void evalStrncpy(CheckerContext &C, const CallExpr *CE) const;
    void evalStpcpy(CheckerContext &C, const CallExpr *CE) const;
    void evalStrlcpy(CheckerContext &C, const CallExpr *CE) const;
    void evalStrcpyCommon(CheckerContext &C, const CallExpr *CE, bool ReturnEnd,
                            bool IsBounded, ConcatFnKind appendK,
                            bool returnPtr = true) const;

    void evalStrcat(CheckerContext &C, const CallExpr *CE) const;
    void evalStrncat(CheckerContext &C, const CallExpr *CE) const;
    void evalStrlcat(CheckerContext &C, const CallExpr *CE) const;

    void evalStrcmp(CheckerContext &C, const CallExpr *CE) const;
    void evalStrncmp(CheckerContext &C, const CallExpr *CE) const;
    void evalStrcasecmp(CheckerContext &C, const CallExpr *CE) const;
    void evalStrncasecmp(CheckerContext &C, const CallExpr *CE) const;
    void evalStrcmpCommon(CheckerContext &C,
                            const CallExpr *CE,
                            bool IsBounded = false,
                            bool IgnoreCase = false) const;

    void evalStrsep(CheckerContext &C, const CallExpr *CE) const;

    void evalStdCopy(CheckerContext &C, const CallExpr *CE) const;
    void evalStdCopyBackward(CheckerContext &C, const CallExpr *CE) const;
    void evalStdCopyCommon(CheckerContext &C, const CallExpr *CE) const;
    void evalMemset(CheckerContext &C, const CallExpr *CE) const;
    void evalBzero(CheckerContext &C, const CallExpr *CE) const;

    // Utility methods
    std::pair<ProgramStateRef , ProgramStateRef >
    static assumeZero(CheckerContext &C,
                        ProgramStateRef state, SVal V, QualType Ty);

    static ProgramStateRef setCStringLength(ProgramStateRef state,
                                                const MemRegion *MR,
                                                SVal strLength);
    static SVal getCStringLengthForRegion(CheckerContext &C,
                                            ProgramStateRef &state,
                                            const Expr *Ex,
                                            const MemRegion *MR,
                                            bool hypothetical);
    SVal getCStringLength(CheckerContext &C,
                            ProgramStateRef &state,
                            const Expr *Ex,
                            SVal Buf,
                            bool hypothetical = false) const;

    const StringLiteral *getCStringLiteral(CheckerContext &C,
                                            ProgramStateRef &state,
                                            const Expr *expr,
                                            SVal val) const;

    static ProgramStateRef InvalidateBuffer(CheckerContext &C,
                                            ProgramStateRef state,
                                            const Expr *Ex, SVal V,
                                            bool IsSourceBuffer,
                                            const Expr *Size);

    static bool SummarizeRegion(raw_ostream &os, ASTContext &Ctx,
                                const MemRegion *MR);

    static bool memsetAux(const Expr *DstBuffer, SVal CharE,
                            const Expr *Size, CheckerContext &C,
                            ProgramStateRef &State);

    // Re-usable checks
    ProgramStateRef checkNonNull(CheckerContext &C, ProgramStateRef State,
                                AnyArgExpr Arg, SVal l) const;
    ProgramStateRef CheckLocation(CheckerContext &C, ProgramStateRef state,
                                    AnyArgExpr Buffer, SVal Element,
                                    AccessKind Access) const;
    ProgramStateRef CheckBufferAccess(CheckerContext &C, ProgramStateRef State,
                                        AnyArgExpr Buffer, SizeArgExpr Size,
                                        AccessKind Access) const;
    ProgramStateRef CheckOverlap(CheckerContext &C, ProgramStateRef state,
                                SizeArgExpr Size, AnyArgExpr First,
                                AnyArgExpr Second) const;
    void emitOverlapBug(CheckerContext &C,
                        ProgramStateRef state,
                        const Stmt *First,
                        const Stmt *Second) const;

    void emitNullArgBug(CheckerContext &C, ProgramStateRef State, const Stmt *S,
                        StringRef WarningMsg) const;
    void emitOutOfBoundsBug(CheckerContext &C, ProgramStateRef State,
                            const Stmt *S, StringRef WarningMsg) const;
    void emitNotCStringBug(CheckerContext &C, ProgramStateRef State,
                            const Stmt *S, StringRef WarningMsg) const;
    void emitAdditionOverflowBug(CheckerContext &C, ProgramStateRef State) const;

    ProgramStateRef checkAdditionOverflow(CheckerContext &C,
                                                ProgramStateRef state,
                                                NonLoc left,
                                                NonLoc right) const;

    // Return true if the destination buffer of the copy function may be in bound.
    // Expects SVal of Size to be positive and unsigned.
    // Expects SVal of FirstBuf to be a FieldRegion.
    static bool IsFirstBufInBound(CheckerContext &C,
                                    ProgramStateRef state,
                                    const Expr *FirstBuf,
                                    const Expr *Size);
    };

} // end namespace ento
} // end namespace clang

#endif