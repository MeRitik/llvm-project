//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "DependentBaseLookupCheck.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void DependentBaseLookupCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      callExpr(callee(declRefExpr(to(functionDecl(unless(cxxMethodDecl()))))
                          .bind("global-callee")),
               hasAncestor(cxxRecordDecl().bind("templated-derived")),
               unless(hasAncestor(cxxDependentScopeMemberExpr())),
               unless(hasAncestor(memberExpr())))
          .bind("unqualified-call"),
      this);
}

void DependentBaseLookupCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Call = Result.Nodes.getNodeAs<CallExpr>("unqualified-call");
  const auto *FD = Result.Nodes.getNodeAs<FunctionDecl>("global-callee");
  const auto *RD = Result.Nodes.getNodeAs<CXXRecordDecl>("templated-derived");
  if (!Call || !FD || !RD)
    return;

  if (!RD->hasAnyDependentBases())
    return;

  diag(Call->getExprLoc(),
       "unqualified lookup in a class with dependent base resolved to %0; "
       "use qualification or this->")
      << FD;
}

} // namespace clang::tidy::hsc
