//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UnusedVariableCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
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
         (Node.hasGlobalStorage() && (Node.getStorageClass() == SC_Static ||
                                      Node.isInAnonymousNamespace()));
}

static const Expr *ignoreParenImpCastAncestors(const Expr *E,
                                               ASTContext &Context) {
  const Expr *Current = E;
  while (Current) {
    auto Parents = Context.getParents(*Current);
    if (Parents.empty())
      break;

    const auto *ParentExpr = Parents[0].get<Expr>();
    if (!ParentExpr)
      break;

    if (!isa<ImplicitCastExpr>(ParentExpr) && !isa<ParenExpr>(ParentExpr))
      break;

    Current = ParentExpr;
  }
  return Current;
}

static bool isWriteOnlyReference(const DeclRefExpr *Ref, ASTContext &Context) {
  const Expr *UseExpr = ignoreParenImpCastAncestors(Ref, Context);
  if (!UseExpr)
    return false;

  auto Parents = Context.getParents(*UseExpr);
  if (Parents.empty())
    return false;

  if (const auto *BO = Parents[0].get<BinaryOperator>()) {
    if ((BO->isAssignmentOp() || BO->isCompoundAssignmentOp()) &&
        BO->getLHS()->IgnoreParenImpCasts() == UseExpr)
      return true;
  }

  if (const auto *UO = Parents[0].get<UnaryOperator>()) {
    if ((UO->getOpcode() == UO_PostInc || UO->getOpcode() == UO_PostDec ||
         UO->getOpcode() == UO_PreInc || UO->getOpcode() == UO_PreDec) &&
        UO->getSubExpr()->IgnoreParenImpCasts() == UseExpr)
      return true;
  }

  return false;
}

static bool hasMeaningfulUseInStmt(const Stmt *S, const VarDecl *Target,
                                   ASTContext &Context) {
  if (!S)
    return false;

  if (const auto *Ref = dyn_cast<DeclRefExpr>(S)) {
    if (Ref->getDecl() == Target && !isWriteOnlyReference(Ref, Context))
      return true;
  }

  for (const Stmt *Child : S->children()) {
    if (hasMeaningfulUseInStmt(Child, Target, Context))
      return true;
  }

  return false;
}

static bool shouldDiagnoseLocal(const VarDecl *VD, ASTContext &Context) {
  const auto *Ctx = VD->getDeclContext();
  const auto *BodyDecl = dyn_cast_or_null<Decl>(Ctx);
  if (!BodyDecl)
    return !VD->isReferenced();

  const Stmt *Body = nullptr;
  if (const auto *FD = dyn_cast<FunctionDecl>(BodyDecl))
    Body = FD->getBody();
  else if (const auto *BD = dyn_cast<BlockDecl>(BodyDecl))
    Body = BD->getBody();
  else if (const auto *CD = dyn_cast<CapturedDecl>(BodyDecl))
    Body = CD->getBody();

  if (!Body)
    return !VD->isReferenced();

  return !hasMeaningfulUseInStmt(Body, VD, Context);
}

} // namespace

void UnusedVariableCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(varDecl(isDefinition(), isLimitedVisibility(),
                             unless(isExceptionVariable()),
                             unless(hasAttr(attr::Unused)),
                             unless(hasType(hasUserProvidedCtorOrDtor())))
                         .bind("unused-var"),
                     this);
}

void UnusedVariableCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *VD = Result.Nodes.getNodeAs<VarDecl>("unused-var");
  if (!VD || !Result.Context)
    return;

  bool ShouldDiagnose = false;
  if (VD->isLocalVarDecl())
    ShouldDiagnose = shouldDiagnoseLocal(VD, *Result.Context);
  else
    ShouldDiagnose = !VD->isReferenced();

  if (!ShouldDiagnose)
    return;

  diag(VD->getLocation(), "variable %0 is never used") << VD;
}

} // namespace clang::tidy::hsc
