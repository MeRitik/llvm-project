//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UnusedVariableCheck.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

AST_MATCHER(QualType, hasUserProvidedCtorOrDtor) {
  const auto *RD = Node->getAsCXXRecordDecl();
  if (!RD)
    return false;

  RD = RD->getDefinition();
  if (!RD)
    return false;

  if (const auto *Dtor = RD->getDestructor()) {
    if (Dtor->isUserProvided())
      return true;
  }

  for (const auto *Ctor : RD->ctors())
    if (Ctor->isUserProvided())
      return true;

  return false;
}

AST_MATCHER(VarDecl, isLimitedVisibility) {
  return Node.isLocalVarDecl() ||
         (Node.hasGlobalStorage() && Node.hasInternalLinkage());
}

} // namespace

void UnusedVariableCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(varDecl(isDefinition(), isLimitedVisibility(),
                             unless(isExceptionVariable()),
                             unless(isReferenced()),
                             unless(hasAttr(attr::Unused)),
                             unless(hasType(hasUserProvidedCtorOrDtor())))
                         .bind("unused-var"),
                     this);
}

void UnusedVariableCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *VD = Result.Nodes.getNodeAs<VarDecl>("unused-var");
  if (!VD)
    return;

  diag(VD->getLocation(), "variable %0 is never used") << VD;
}

} // namespace clang::tidy::hsc
