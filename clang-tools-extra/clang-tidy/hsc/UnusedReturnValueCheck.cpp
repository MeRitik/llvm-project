//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UnusedReturnValueCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/OperatorKinds.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

// HSCAA.1.2 exempts overloaded operators used with operator syntax, except
// function-call syntax (operator()), which must still be checked.
AST_MATCHER(CXXOperatorCallExpr, isExemptOperatorSyntax) {
  return Node.getOperator() != OO_Call;
}

} // namespace

void UnusedReturnValueCheck::registerMatchers(MatchFinder *Finder) {
  auto MatchedCallExpr =
      expr(callExpr(unless(hasType(voidType())),
                    unless(cxxOperatorCallExpr(isExemptOperatorSyntax())),
                    unless(hasParent(castExpr(hasCastKind(CK_ToVoid)))))
               .bind("discarded-call"));

  auto UnusedInCompoundStmt =
      compoundStmt(forEach(MatchedCallExpr), unless(hasParent(stmtExpr())));
  auto UnusedInIfStmt =
      ifStmt(eachOf(hasThen(MatchedCallExpr), hasElse(MatchedCallExpr)));
  auto UnusedInWhileStmt = whileStmt(hasBody(MatchedCallExpr));
  auto UnusedInDoStmt = doStmt(hasBody(MatchedCallExpr));
  auto UnusedInForStmt =
      forStmt(eachOf(hasLoopInit(MatchedCallExpr),
                     hasIncrement(MatchedCallExpr), hasBody(MatchedCallExpr)));
  auto UnusedInRangeForStmt = cxxForRangeStmt(hasBody(MatchedCallExpr));
  auto UnusedInCaseStmt = switchCase(forEach(MatchedCallExpr));

  Finder->addMatcher(
      stmt(anyOf(UnusedInCompoundStmt, UnusedInIfStmt, UnusedInWhileStmt,
                 UnusedInDoStmt, UnusedInForStmt, UnusedInRangeForStmt,
                 UnusedInCaseStmt)),
      this);
}

void UnusedReturnValueCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Matched = Result.Nodes.getNodeAs<CallExpr>("discarded-call");
  if (!Matched)
    return;

  diag(Matched->getBeginLoc(),
       "the value returned by this function should be used")
      << Matched->getSourceRange();

  diag(Matched->getBeginLoc(),
       "cast the expression to void to indicate that discarding the value is "
       "intentional",
       DiagnosticIDs::Note);
}

} // namespace clang::tidy::hsc
