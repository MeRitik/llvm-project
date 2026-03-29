//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "EscapingLocalAddressCheck.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

#include "llvm/ADT/SmallVector.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

const VarDecl *getSourceLocalVar(const Expr *RHS) {
  if (!RHS)
    return nullptr;

  RHS = RHS->IgnoreParenImpCasts();

  if (const auto *UO = dyn_cast<UnaryOperator>(RHS)) {
    if (UO->getOpcode() != UO_AddrOf)
      return nullptr;

    const auto *DRE =
        dyn_cast<DeclRefExpr>(UO->getSubExpr()->IgnoreParenImpCasts());
    const auto *VD = DRE ? dyn_cast<VarDecl>(DRE->getDecl()) : nullptr;
    return (VD && VD->hasLocalStorage()) ? VD : nullptr;
  }

  // Array-to-pointer decay (`ptr = arr;`) also escapes local storage.
  if (const auto *DRE = dyn_cast<DeclRefExpr>(RHS)) {
    const auto *VD = dyn_cast<VarDecl>(DRE->getDecl());
    if (VD && VD->hasLocalStorage() && VD->getType()->isArrayType())
      return VD;
  }

  return nullptr;
}

bool containsLoc(const Stmt *S, SourceLocation Loc, const SourceManager &SM) {
  if (!S || Loc.isInvalid())
    return false;

  SourceLocation Begin = S->getBeginLoc();
  SourceLocation End = S->getEndLoc();
  if (Begin.isInvalid() || End.isInvalid())
    return false;

  return !SM.isBeforeInTranslationUnit(Loc, Begin) &&
         !SM.isBeforeInTranslationUnit(End, Loc);
}

bool buildCompoundPath(const Stmt *S, SourceLocation Loc,
                       const SourceManager &SM,
                       llvm::SmallVectorImpl<const CompoundStmt *> &Path) {
  if (!containsLoc(S, Loc, SM))
    return false;

  if (const auto *CS = dyn_cast<CompoundStmt>(S))
    Path.push_back(CS);

  for (const Stmt *Child : S->children())
    if (Child && buildCompoundPath(Child, Loc, SM, Path))
      return true;

  return true;
}

bool isStrictPrefix(llvm::SmallVectorImpl<const CompoundStmt *> &Outer,
                    llvm::SmallVectorImpl<const CompoundStmt *> &Inner) {
  if (Outer.size() >= Inner.size())
    return false;
  for (size_t I = 0; I < Outer.size(); ++I)
    if (Outer[I] != Inner[I])
      return false;
  return true;
}

bool targetOutlivesLocal(const VarDecl *Target, const VarDecl *Local,
                         const BinaryOperator *Assign,
                         const MatchFinder::MatchResult &Result) {
  if (!Target || !Local || !Assign || !Result.Context || !Result.SourceManager)
    return false;

  if (Target->hasGlobalStorage())
    return true;

  if (!Target->isLocalVarDecl() || !Local->isLocalVarDecl())
    return false;

  const auto *Ctx = dyn_cast<FunctionDecl>(Target->getDeclContext());
  if (!Ctx || !Ctx->doesThisDeclarationHaveABody())
    return Target->getDeclContext() != Local->getDeclContext();

  const Stmt *Body = Ctx->getBody();
  if (!Body)
    return Target->getDeclContext() != Local->getDeclContext();

  llvm::SmallVector<const CompoundStmt *, 8> TargetPath;
  llvm::SmallVector<const CompoundStmt *, 8> LocalPath;

  if (!buildCompoundPath(Body, Target->getLocation(), *Result.SourceManager,
                         TargetPath) ||
      !buildCompoundPath(Body, Local->getLocation(), *Result.SourceManager,
                         LocalPath))
    return Target->getDeclContext() != Local->getDeclContext();

  return isStrictPrefix(TargetPath, LocalPath);
}

} // namespace

void EscapingLocalAddressCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      binaryOperator(isAssignmentOperator(),
                     hasLHS(ignoringParenImpCasts(declRefExpr(
                         to(varDecl(hasType(pointerType())).bind("target"))))))
          .bind("assign"),
      this);
}

void EscapingLocalAddressCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Target = Result.Nodes.getNodeAs<VarDecl>("target");
  const auto *Assign = Result.Nodes.getNodeAs<BinaryOperator>("assign");
  if (!Target || !Assign)
    return;

  const VarDecl *Local = getSourceLocalVar(Assign->getRHS());
  if (!Local)
    return;

  if (!targetOutlivesLocal(Target, Local, Assign, Result))
    return;

  diag(Assign->getOperatorLoc(),
       "assigning address of automatic-storage object %0 to pointer %1 with "
       "greater lifetime may create a dangling pointer")
      << Local << Target;
}

} // namespace clang::tidy::hsc
