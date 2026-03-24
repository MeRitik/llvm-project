//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UnusedTypeCheck.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

AST_MATCHER(CXXRecordDecl, isRecordInLimitedScope) {
  // Check if in block scope or anonymous namespace
  const DeclContext *DC = Node.getDeclContext();

  // Block scope (function scope)
  if (isa<FunctionDecl>(DC))
    return true;

  // Anonymous namespace
  if (const auto *NS = dyn_cast<NamespaceDecl>(DC)) {
    if (NS->isAnonymousNamespace())
      return true;
  }

  return false;
}

AST_MATCHER(TypeAliasDecl, isAliasInLimitedScope) {
  // Check if in block scope or anonymous namespace
  const DeclContext *DC = Node.getDeclContext();

  // Block scope (function scope)
  if (isa<FunctionDecl>(DC))
    return true;

  // Anonymous namespace
  if (const auto *NS = dyn_cast<NamespaceDecl>(DC)) {
    if (NS->isAnonymousNamespace())
      return true;
  }

  return false;
}

AST_MATCHER(CXXRecordDecl, isUnusedRecord) {
  return !Node.isReferenced() && !Node.isLambda();
}

AST_MATCHER(TypeAliasDecl, isUnusedAlias) { return !Node.isReferenced(); }

} // namespace

void UnusedTypeCheck::registerMatchers(MatchFinder *Finder) {
  // Match unused class/struct declarations in limited scope
  Finder->addMatcher(cxxRecordDecl(isRecordInLimitedScope(), isUnusedRecord(),
                                   unless(hasAttr(attr::Unused)))
                         .bind("unused-type"),
                     this);

  // Match unused type aliases in limited scope
  Finder->addMatcher(typeAliasDecl(isAliasInLimitedScope(), isUnusedAlias(),
                                   unless(hasAttr(attr::Unused)))
                         .bind("unused-alias"),
                     this);
}

void UnusedTypeCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto *RD = Result.Nodes.getNodeAs<CXXRecordDecl>("unused-type")) {
    diag(RD->getLocation(), "type %0 is never used") << RD;
    return;
  }

  if (const auto *TAD = Result.Nodes.getNodeAs<TypeAliasDecl>("unused-alias")) {
    diag(TAD->getLocation(), "type alias %0 is never used") << TAD;
    return;
  }
}

std::optional<TraversalKind> UnusedTypeCheck::getCheckTraversalKind() const {
  return TK_IgnoreUnlessSpelledInSource;
}

} // namespace clang::tidy::hsc
