//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "EscapingLocalAddressCheck.h"
#include "clang/AST/Decl.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void EscapingLocalAddressCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      binaryOperator(isAssignmentOperator(),
                     hasLHS(ignoringParenImpCasts(declRefExpr(
                         to(varDecl(hasType(pointerType())).bind("target"))))),
                     hasRHS(ignoringParenImpCasts(unaryOperator(
                         hasOperatorName("&"),
                         hasUnaryOperand(declRefExpr(
                             to(varDecl(hasLocalStorage()).bind("local"))))))))
          .bind("assign"),
      this);
}

void EscapingLocalAddressCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Target = Result.Nodes.getNodeAs<VarDecl>("target");
  const auto *Local = Result.Nodes.getNodeAs<VarDecl>("local");
  const auto *Assign = Result.Nodes.getNodeAs<BinaryOperator>("assign");
  if (!Target || !Local || !Assign)
    return;

  bool EscapesLifetime = false;

  if (Target->hasGlobalStorage()) {
    EscapesLifetime = true;
  } else if (Target->isLocalVarDecl() && Local->isLocalVarDecl()) {
    if (Target->getDeclContext() != Local->getDeclContext())
      EscapesLifetime = true;
  }

  if (!EscapesLifetime)
    return;

  diag(Assign->getOperatorLoc(),
       "assigning address of automatic-storage object %0 to pointer %1 with "
       "greater lifetime may create a dangling pointer")
      << Local << Target;
}

} // namespace clang::tidy::hsc
