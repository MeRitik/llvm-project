#include "ReinterpretCastCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

bool isStdByteType(QualType Type) {
  Type = Type.getCanonicalType().getUnqualifiedType();

  if (const auto *Enum = Type->getAs<EnumType>()) {
    const EnumDecl *Decl = Enum->getDecl();
    return Decl && Decl->getQualifiedNameAsString() == "std::byte";
  }

  return false;
}

bool isAllowedByteLikeType(QualType Type) {
  Type = Type.getCanonicalType().getUnqualifiedType();

  return Type->isVoidType() || Type->isCharType() ||
         Type->isSpecificBuiltinType(BuiltinType::UChar) || isStdByteType(Type);
}

bool isObjectPointerType(QualType Type) {
  if (!Type->isPointerType() || Type->isMemberPointerType())
    return false;

  QualType Pointee = Type->getPointeeType();
  return !Pointee.isNull() && !Pointee->isFunctionType();
}

bool isAllowedObjectPtrToBytePtr(QualType FromType, QualType ToType) {
  if (!isObjectPointerType(FromType))
    return false;

  if (!ToType->isPointerType() || ToType->isMemberPointerType())
    return false;

  return isAllowedByteLikeType(ToType->getPointeeType());
}

bool isAllowedPointerToInteger(ASTContext &Context, QualType FromType,
                               QualType ToType) {
  if (!FromType->isPointerType() || FromType->isMemberPointerType())
    return false;

  if (!ToType->isIntegerType())
    return false;

  uint64_t ToWidth = Context.getTypeSize(ToType);
  uint64_t PtrWidth = Context.getTypeSize(Context.getUIntPtrType());

  return ToWidth >= PtrWidth;
}

bool isForbiddenReinterpretCast(ASTContext &Context,
                                const CXXReinterpretCastExpr *CastExpr) {
  if (!CastExpr)
    return false;

  const Expr *SubExpr = CastExpr->getSubExprAsWritten();
  if (!SubExpr)
    return false;

  QualType FromType = SubExpr->IgnoreParenImpCasts()->getType();
  QualType ToType = CastExpr->getTypeAsWritten();

  if (FromType.isNull() || ToType.isNull() || FromType->isDependentType() ||
      ToType->isDependentType())
    return false;

  if (isAllowedObjectPtrToBytePtr(FromType, ToType))
    return false;

  if (isAllowedPointerToInteger(Context, FromType, ToType))
    return false;

  return true;
}

} // namespace

void ReinterpretCastCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxReinterpretCastExpr().bind("cast"), this);
}

void ReinterpretCastCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *CastExpr = Result.Nodes.getNodeAs<CXXReinterpretCastExpr>("cast");

  if (!CastExpr || !Result.Context)
    return;

  if (!isForbiddenReinterpretCast(*Result.Context, CastExpr))
    return;

  diag(CastExpr->getExprLoc(), "reinterpret_cast shall not be used");
}

} // namespace clang::tidy::hsc
