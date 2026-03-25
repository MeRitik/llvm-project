#include "VirtualMemberPointerCmpCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void VirtualMemberPointerCmpCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      binaryOperator(
          anyOf(hasOperatorName("=="), hasOperatorName("!=")),
          hasEitherOperand(declRefExpr(to(cxxMethodDecl(isVirtual())))))
          .bind("cmp"),
      this);
}

void VirtualMemberPointerCmpCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *Cmp = Result.Nodes.getNodeAs<BinaryOperator>("cmp");
  if (!Cmp)
    return;

  diag(Cmp->getExprLoc(),
       "comparing virtual member function pointers with non-nullptr is "
       "undefined behaviour");
}

} // namespace clang::tidy::hsc
