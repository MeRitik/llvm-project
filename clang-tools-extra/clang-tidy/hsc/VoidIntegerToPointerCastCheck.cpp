#include "VoidIntegerToPointerCastCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {
namespace {

bool isVoidPointerType(QualType Type) {
  Type = Type.getCanonicalType();
  if (!Type->isPointerType() || Type->isMemberPointerType())
    return false;

  QualType Pointee = Type->getPointeeType();
  if (Pointee.isNull())
    return false;

  return Pointee.getCanonicalType().getUnqualifiedType()->isVoidType();
}

bool isForbiddenVoidOrIntegerToPointerCast(ASTContext &Context,
                                           const ExplicitCastExpr *CastExpr) {
  if (!CastExpr)
    return false;

  const Expr *SubExpr = CastExpr->getSubExprAsWritten();
  if (!SubExpr)
    return false;

  QualType ToType = CastExpr->getTypeAsWritten();
  if (ToType.isNull() || ToType->isDependentType())
    return false;

  if (!ToType->isPointerType() || ToType->isMemberPointerType())
    return false;

  QualType FromType = SubExpr->IgnoreParenImpCasts()->getType();
  if (FromType.isNull() || FromType->isDependentType())
    return false;

  if (FromType->isNullPtrType())
    return false;

  // Casting an integer or enum value to ANY pointer type is forbidden.
  if (FromType->isIntegerType() || FromType->isEnumeralType())
    return true;

  // Casting from void* to a non-void pointer is forbidden.
  if (isVoidPointerType(FromType) && !isVoidPointerType(ToType))
    return true;

  // Otherwise, rule does not apply.
  (void)Context;
  return false;
}

} // namespace

void VoidIntegerToPointerCastCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      explicitCastExpr(unless(anyOf(cxxConstCastExpr(), cxxDynamicCastExpr())))
          .bind("cast"),
      this);
}

void VoidIntegerToPointerCastCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *CastExpr = Result.Nodes.getNodeAs<ExplicitCastExpr>("cast");
  if (!Result.Context ||
      !isForbiddenVoidOrIntegerToPointerCast(*Result.Context, CastExpr))
    return;

  diag(CastExpr->getExprLoc(),
       "casting from integer, enum, or void* to a pointer type "
       "type is prohibited");
}

} // namespace clang::tidy::hsc
