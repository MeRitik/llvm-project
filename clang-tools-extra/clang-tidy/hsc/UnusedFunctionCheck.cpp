//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UnusedFunctionCheck.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

AST_MATCHER(FunctionDecl, hasLimitedVisibility) {
  // Check if static function
  if (Node.getStorageClass() == SC_Static)
    return true;

  // Check if in anonymous namespace
  if (const auto *NS = dyn_cast<NamespaceDecl>(Node.getDeclContext())) {
    if (NS->isAnonymousNamespace())
      return true;
  }

  // Check if private member function (non-virtual)
  if (const auto *MD = dyn_cast<CXXMethodDecl>(&Node)) {
    if (MD->getAccess() == AS_private && !MD->isVirtual())
      return true;
  }

  return false;
}

AST_MATCHER(FunctionDecl, isUnusedFunction) {
  // Virtual functions are excluded
  if (const auto *MD = dyn_cast<CXXMethodDecl>(&Node)) {
    if (MD->isVirtual())
      return false;
    // Special member functions are excluded
    if (isa<CXXConstructorDecl>(MD) || isa<CXXDestructorDecl>(MD))
      return false;
    // Also exclude assignment and conversion operators
    if (isa<CXXConversionDecl>(MD))
      return false;
    if (const auto *Op = dyn_cast<CXXMethodDecl>(MD)) {
      if (Op->isCopyAssignmentOperator() || Op->isMoveAssignmentOperator())
        return false;
    }
  }

  return !Node.isReferenced();
}

} // namespace

void UnusedFunctionCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(functionDecl(hasLimitedVisibility(), isUnusedFunction(),
                                  unless(hasAttr(attr::Unused)))
                         .bind("unused-func"),
                     this);
}

void UnusedFunctionCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *FD = Result.Nodes.getNodeAs<FunctionDecl>("unused-func");
  if (!FD)
    return;

  diag(FD->getLocation(), "function %0 is never used") << FD;
}

std::optional<TraversalKind>
UnusedFunctionCheck::getCheckTraversalKind() const {
  return TK_IgnoreUnlessSpelledInSource;
}

} // namespace clang::tidy::hsc
