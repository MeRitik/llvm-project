//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ExplicitBaseInitCheck.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/ADT/SmallPtrSet.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

static const Type *canonicalType(const TypeSourceInfo *TSI,
                                 ASTContext &Context) {
  if (!TSI)
    return nullptr;
  return Context.getCanonicalType(TSI->getType()).getTypePtrOrNull();
}

static bool isExplicitlyInitialized(const CXXConstructorDecl *Ctor,
                                    const Type *BaseCanon,
                                    ASTContext &Context) {
  for (const CXXCtorInitializer *Init : Ctor->inits()) {
    if (!Init->isBaseInitializer())
      continue;
    const Type *InitCanon = canonicalType(Init->getTypeSourceInfo(), Context);
    if (InitCanon == BaseCanon)
      return true;
  }
  return false;
}

} // namespace

void ExplicitBaseInitCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxConstructorDecl(isDefinition(), unless(isImplicit()))
                         .bind("constructor"),
                     this);
}

void ExplicitBaseInitCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("constructor");
  if (!Ctor)
    return;

  if (Ctor->isDelegatingConstructor())
    return;

  const auto *RD = Ctor->getParent();
  if (!RD)
    return;

  llvm::SmallPtrSet<const CXXRecordDecl *, 8> SeenBases;

  auto CheckBase = [&](const CXXBaseSpecifier &BaseSpec) {
    const auto *BaseRD = BaseSpec.getType()->getAsCXXRecordDecl();
    if (!BaseRD)
      return;

    BaseRD = BaseRD->getDefinition();
    if (!BaseRD)
      return;

    if (!SeenBases.insert(BaseRD).second)
      return;

    if (BaseRD->isEmpty())
      return;

    const Type *BaseCanon =
        Result.Context->getCanonicalType(BaseSpec.getType()).getTypePtrOrNull();
    if (!BaseCanon)
      return;

    if (!isExplicitlyInitialized(Ctor, BaseCanon, *Result.Context)) {
      diag(Ctor->getLocation(),
           "constructor does not explicitly initialize base class %0")
          << BaseRD;
    }
  };

  for (const CXXBaseSpecifier &Base : RD->bases())
    CheckBase(Base);
  for (const CXXBaseSpecifier &VBase : RD->vbases())
    CheckBase(VBase);
}

} // namespace clang::tidy::hsc
