#include "VirtualBaseToDerivedCastCheck.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {
namespace {

const CXXRecordDecl *getPointeeOrReferredRecord(QualType Type) {
  if (Type->isPointerType())
    Type = Type->getPointeeType();
  else if (Type->isReferenceType())
    Type = Type->getPointeeType();
  else
    return nullptr;

  const auto *RecordDecl =
      Type.getCanonicalType().getUnqualifiedType()->getAsCXXRecordDecl();
  return RecordDecl ? RecordDecl->getCanonicalDecl() : nullptr;
}

bool isForbiddenVirtualBaseToDerivedCast(const ExplicitCastExpr *CastExpr) {
  if (!CastExpr || isa<CXXDynamicCastExpr>(CastExpr))
    return false;

  if (!isa<CXXReinterpretCastExpr, CXXStaticCastExpr, CStyleCastExpr>(CastExpr))
    return false;

  const Expr *SourceExpr = CastExpr->getSubExprAsWritten();
  if (!SourceExpr)
    return false;

  const CXXRecordDecl *SourceRecord = getPointeeOrReferredRecord(
      SourceExpr->IgnoreParenImpCasts()->getType().getCanonicalType());

  const CXXRecordDecl *TargetRecord =
      getPointeeOrReferredRecord(CastExpr->getType().getCanonicalType());

  if (!SourceRecord || !TargetRecord || SourceRecord == TargetRecord)
    return false;

  return TargetRecord->isVirtuallyDerivedFrom(SourceRecord);
}

} // namespace

void VirtualBaseToDerivedCastCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(explicitCastExpr().bind("cast"), this);
}

void VirtualBaseToDerivedCastCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *CastExpr = Result.Nodes.getNodeAs<ExplicitCastExpr>("cast");
  if (!isForbiddenVirtualBaseToDerivedCast(CastExpr))
    return;

  diag(CastExpr->getExprLoc(),
       "casting from a virtual base class to a derived class must use "
       "dynamic_cast");
}

} // namespace clang::tidy::hsc
