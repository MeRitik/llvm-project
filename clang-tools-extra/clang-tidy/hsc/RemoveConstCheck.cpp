#include "RemoveConstCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void RemoveConstCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxConstCastExpr().bind("const_cast"), this);
}

static bool removesCVQualifiers(QualType From, QualType To) {
  From = From.getCanonicalType();
  To = To.getCanonicalType();

  // Handle pointer types
  if (From->isPointerType() && To->isPointerType()) {
    QualType FromPointee = From->getPointeeType();
    QualType ToPointee = To->getPointeeType();

    return (FromPointee.isConstQualified() && !ToPointee.isConstQualified()) ||
           (FromPointee.isVolatileQualified() &&
            !ToPointee.isVolatileQualified());
  }

  // Handle reference types
  if (From->isReferenceType() && To->isReferenceType()) {
    QualType FromRef = From->getPointeeType();
    QualType ToRef = To->getPointeeType();

    return (FromRef.isConstQualified() && !ToRef.isConstQualified()) ||
           (FromRef.isVolatileQualified() && !ToRef.isVolatileQualified());
  }

  return false;
}

void RemoveConstCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *ConstCast =
      Result.Nodes.getNodeAs<CXXConstCastExpr>("const_cast");
  if (!ConstCast)
    return;

  QualType FromType = ConstCast->getSubExpr()->getType();
  QualType ToType = ConstCast->getTypeAsWritten();

  if (!removesCVQualifiers(FromType, ToType))
    return;

  diag(ConstCast->getExprLoc(),
       "cast removes const/volatile qualification from the referenced type");
}

} // namespace clang::tidy::hsc
