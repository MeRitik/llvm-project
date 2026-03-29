#include "CStyleFunctionalCastCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ExprCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

// Allow (void)expr
bool isPermittedVoidCStyleCast(const CStyleCastExpr *CastExpr) {
  if (!CastExpr)
    return false;

  return CastExpr->getCastKind() == CK_ToVoid;
}

bool isForbiddenFunctionalCast(const CXXFunctionalCastExpr *CastExpr) {
  if (!CastExpr)
    return false;

  if (CastExpr->isListInitialization())
    return false;

  if (CastExpr->getCastKind() == CK_ConstructorConversion)
    return false;

  const QualType T = CastExpr->getTypeAsWritten();
  if (T.isNull() || T->isDependentType())
    return false;

  return true;
}

} // namespace

void CStyleFunctionalCastCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cStyleCastExpr().bind("cstyle"), this);
  Finder->addMatcher(cxxFunctionalCastExpr().bind("functional"), this);
}

void CStyleFunctionalCastCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto *CStyle = Result.Nodes.getNodeAs<CStyleCastExpr>("cstyle")) {
    if (isPermittedVoidCStyleCast(CStyle))
      return;

    diag(CStyle->getBeginLoc(),
         "C-style casts shall not be used; use static_cast, const_cast, "
         "reinterpret_cast, or dynamic_cast instead");
    return;
  }

  const auto *Functional =
      Result.Nodes.getNodeAs<CXXFunctionalCastExpr>("functional");

  if (!Functional)
    return;

  if (!isForbiddenFunctionalCast(Functional))
    return;

  diag(Functional->getBeginLoc(),
       "functional-notation casts shall not be used; prefer explicit cast "
       "operators or brace initialization");
}

} // namespace clang::tidy::hsc
