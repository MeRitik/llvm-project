#include "DynamicTypeInConstructorCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/ExprCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void DynamicTypeInConstructorCheck::registerMatchers(MatchFinder *Finder) {
  const auto InCtorOrDtor = anyOf(hasAncestor(cxxConstructorDecl()),
                                  hasAncestor(cxxDestructorDecl()));

  Finder->addMatcher(expr(InCtorOrDtor).bind("expr-in-special-member"), this);
  Finder->addMatcher(cxxDynamicCastExpr(InCtorOrDtor).bind("dynamic-cast"),
                     this);
  Finder->addMatcher(
      cxxMemberCallExpr(InCtorOrDtor, callee(cxxMethodDecl(isVirtual())))
          .bind("virtual-call"),
      this);
}

void DynamicTypeInConstructorCheck::check(
    const MatchFinder::MatchResult &Result) {
  if (const auto *ExprInSpecialMember =
          Result.Nodes.getNodeAs<Expr>("expr-in-special-member")) {
    const auto *TypeId = dyn_cast<CXXTypeidExpr>(ExprInSpecialMember);
    if (!TypeId)
      return;

    if (TypeId->isTypeOperand())
      return;

    const Expr *Operand = TypeId->getExprOperand();
    if (!Operand)
      return;

    const auto *Record = Operand->getType()->getAsCXXRecordDecl();
    if (!Record || !Record->isPolymorphic())
      return;

    diag(TypeId->getExprLoc(),
         "typeid on a polymorphic expression should not be used in a "
         "constructor or destructor");
    return;
  }

  if (const auto *DynCast =
          Result.Nodes.getNodeAs<CXXDynamicCastExpr>("dynamic-cast")) {
    diag(DynCast->getExprLoc(),
         "dynamic_cast should not be used in a constructor or destructor");
    return;
  }

  const auto *VirtualCall =
      Result.Nodes.getNodeAs<CXXMemberCallExpr>("virtual-call");
  if (!VirtualCall)
    return;

  // Qualified calls like Base::f() are not virtual dispatch.
  const Expr *Callee = VirtualCall->getCallee();
  if (const auto *Member =
          dyn_cast_or_null<MemberExpr>(Callee->IgnoreParenImpCasts())) {
    if (Member->hasQualifier())
      return;
  }

  diag(VirtualCall->getExprLoc(),
       "virtual function call should not be used in a constructor or "
       "destructor");
}

} // namespace clang::tidy::hsc
