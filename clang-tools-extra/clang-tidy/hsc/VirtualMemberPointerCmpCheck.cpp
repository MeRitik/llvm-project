#include "VirtualMemberPointerCmpCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

const MemberPointerType *getMemberFunctionPointerType(QualType QT) {
  const auto *MPT = QT->getAs<MemberPointerType>();
  if (!MPT)
    return nullptr;
  if (!MPT->getPointeeType()->isFunctionType())
    return nullptr;
  return MPT;
}

bool isNullPointerComparisonOperand(const Expr *E,
                                    const MatchFinder::MatchResult &Result) {
  if (!E || !Result.Context)
    return false;
  return E->isNullPointerConstant(*Result.Context,
                                  Expr::NPC_ValueDependentIsNull);
}

const CXXMethodDecl *getAddressedMemberMethod(const Expr *E) {
  if (!E)
    return nullptr;

  E = E->IgnoreParenImpCasts();
  const auto *UO = dyn_cast<UnaryOperator>(E);
  if (!UO || UO->getOpcode() != UO_AddrOf)
    return nullptr;

  const auto *ME =
      dyn_cast<MemberExpr>(UO->getSubExpr()->IgnoreParenImpCasts());
  if (ME)
    return dyn_cast<CXXMethodDecl>(ME->getMemberDecl());

  const Expr *SubExpr = UO->getSubExpr()->IgnoreParenImpCasts();
  if (const auto *DRE = dyn_cast<DeclRefExpr>(SubExpr))
    return dyn_cast<CXXMethodDecl>(DRE->getDecl());

  if (const auto *ULE = dyn_cast<UnresolvedLookupExpr>(SubExpr)) {
    if (ULE->getNumDecls() == 1)
      return dyn_cast<CXXMethodDecl>(*ULE->decls_begin());
  }

  return nullptr;
}

bool classHasMatchingVirtualMethod(const CXXRecordDecl *RD,
                                   QualType MemberFunctionType,
                                   ASTContext &Context) {
  if (!RD)
    return false;

  const auto *Definition = RD->getDefinition();
  if (!Definition)
    return true;

  const QualType CanonTarget = Context.getCanonicalType(MemberFunctionType);
  for (const auto *Method : Definition->methods()) {
    if (!Method->isVirtual())
      continue;
    if (Context.hasSameType(Context.getCanonicalType(Method->getType()),
                            CanonTarget))
      return true;
  }

  return false;
}

bool isPotentiallyVirtualMemberFunctionPointer(
    const Expr *E, const MatchFinder::MatchResult &Result) {
  if (!E || !Result.Context)
    return false;

  if (const auto *Method = getAddressedMemberMethod(E))
    return Method->isVirtual();

  const auto *MPT = getMemberFunctionPointerType(E->getType());
  if (!MPT)
    return false;

  const auto *RD = MPT->getMostRecentCXXRecordDecl();
  if (!RD)
    return true;

  return classHasMatchingVirtualMethod(RD, MPT->getPointeeType(),
                                       *Result.Context);
}

} // namespace

void VirtualMemberPointerCmpCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      binaryOperator(anyOf(hasOperatorName("=="), hasOperatorName("!=")))
          .bind("cmp"),
      this);
}

void VirtualMemberPointerCmpCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *Cmp = Result.Nodes.getNodeAs<BinaryOperator>("cmp");
  if (!Cmp)
    return;

  const Expr *LHS = Cmp->getLHS()->IgnoreParenImpCasts();
  const Expr *RHS = Cmp->getRHS()->IgnoreParenImpCasts();

  const auto *LHSMPT = getMemberFunctionPointerType(LHS->getType());
  const auto *RHSMPT = getMemberFunctionPointerType(RHS->getType());
  if (!LHSMPT && !RHSMPT)
    return;

  if (isNullPointerComparisonOperand(LHS, Result) ||
      isNullPointerComparisonOperand(RHS, Result))
    return;

  if (!isPotentiallyVirtualMemberFunctionPointer(LHS, Result) &&
      !isPotentiallyVirtualMemberFunctionPointer(RHS, Result))
    return;

  diag(Cmp->getExprLoc(),
       "comparison of a potentially virtual pointer to member function "
       "shall only be with nullptr");
}

} // namespace clang::tidy::hsc
