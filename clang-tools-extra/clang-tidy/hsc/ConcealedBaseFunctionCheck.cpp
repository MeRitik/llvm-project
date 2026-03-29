//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ConcealedBaseFunctionCheck.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

static bool hasUsingForName(const CXXRecordDecl *RD, DeclarationName Name) {
  for (const Decl *D : RD->decls()) {
    if (const auto *UD = dyn_cast<UsingDecl>(D)) {
      if (UD->getDeclName() == Name)
        return true;
    }
  }
  return false;
}

static bool hasBaseMethodNamed(const CXXRecordDecl *RD, DeclarationName Name,
                               bool AccessiblePath = true) {
  if (!RD)
    return false;

  for (const CXXBaseSpecifier &Base : RD->bases()) {
    const auto *BaseRD = Base.getType()->getAsCXXRecordDecl();
    if (!BaseRD)
      continue;

    const bool NextAccessible =
        AccessiblePath && Base.getAccessSpecifier() != AS_private;

    if (NextAccessible) {
      for (const CXXMethodDecl *M : BaseRD->methods())
        if (M->getDeclName() == Name)
          return true;
    }

    if (hasBaseMethodNamed(BaseRD, Name, NextAccessible))
      return true;
  }

  return false;
}

} // namespace

void ConcealedBaseFunctionCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxMethodDecl(unless(isImplicit())).bind("derived-method"),
                     this);
}

void ConcealedBaseFunctionCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *MD = Result.Nodes.getNodeAs<CXXMethodDecl>("derived-method");
  if (!MD || !MD->getIdentifier())
    return;

  // Diagnose once per method at the first declaration so declaration-only
  // cases are covered without duplicate diagnostics.
  if (!MD->isFirstDecl())
    return;

  if (isa<CXXConstructorDecl>(MD) || isa<CXXDestructorDecl>(MD) ||
      isa<CXXConversionDecl>(MD))
    return;

  if (MD->size_overridden_methods() > 0)
    return;

  const auto *RD = MD->getParent();
  if (!RD)
    return;

  if (hasUsingForName(RD, MD->getDeclName()))
    return;

  if (!hasBaseMethodNamed(RD, MD->getDeclName()))
    return;

  diag(MD->getLocation(),
       "function %0 conceals a function inherited from an accessible base "
       "class")
      << MD;
}

} // namespace clang::tidy::hsc
