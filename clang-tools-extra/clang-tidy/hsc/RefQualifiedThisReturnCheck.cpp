//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "RefQualifiedThisReturnCheck.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

static bool isThisOrSubobjectExpr(const Expr *E) {
  if (!E)
    return false;

  E = E->IgnoreParenImpCasts();

  if (isa<CXXThisExpr>(E))
    return true;

  if (const auto *ME = dyn_cast<MemberExpr>(E)) {
    const Expr *Base = ME->getBase()->IgnoreParenImpCasts();
    if (isa<CXXThisExpr>(Base)) {
      if (const auto *FD = dyn_cast<FieldDecl>(ME->getMemberDecl())) {
        if (FD->getType()->isReferenceType())
          return false;
      }
      return true;
    }
  }

  if (const auto *UO = dyn_cast<UnaryOperator>(E)) {
    if (UO->getOpcode() == UO_Deref) {
      const Expr *Sub = UO->getSubExpr()->IgnoreParenImpCasts();
      if (isa<CXXThisExpr>(Sub))
        return true;
    }

    if (UO->getOpcode() == UO_AddrOf) {
      const Expr *Sub = UO->getSubExpr()->IgnoreParenImpCasts();
      if (const auto *ME = dyn_cast<MemberExpr>(Sub)) {
        const Expr *Base = ME->getBase()->IgnoreParenImpCasts();
        if (isa<CXXThisExpr>(Base))
          return true;
      }
      if (isa<CXXThisExpr>(Sub))
        return true;
    }
  }

  return false;
}

static bool returnsThisOrSubobject(const Stmt *S) {
  if (!S)
    return false;

  if (const auto *RS = dyn_cast<ReturnStmt>(S)) {
    if (isThisOrSubobjectExpr(RS->getRetValue()))
      return true;
  }

  for (const Stmt *Child : S->children())
    if (returnsThisOrSubobject(Child))
      return true;

  return false;
}

} // namespace

void RefQualifiedThisReturnCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxMethodDecl(isDefinition(),
                    returns(anyOf(referenceType(), pointerType())))
          .bind("method"),
      this);
}

void RefQualifiedThisReturnCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *MD = Result.Nodes.getNodeAs<CXXMethodDecl>("method");
  if (!MD || !MD->hasBody())
    return;

  if (MD->isStatic())
    return;

  if (MD->getRefQualifier() == RQ_LValue)
    return;

  if (!returnsThisOrSubobject(MD->getBody()))
    return;

  diag(MD->getLocation(),
       "member function returning this/subobject should be ref-qualified to "
       "avoid dangling references on temporaries")
      << MD;
}

} // namespace clang::tidy::hsc
