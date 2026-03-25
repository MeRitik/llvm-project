#include "DynamicTypeInConstructorCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void DynamicTypeInConstructorCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxConstructorDecl(
          hasBody(compoundStmt(has(callExpr(callee(functionDecl(anyOf(
                                                hasName("typeid"),
                                                hasName("dynamic_cast")))))
                                       .bind("call")))
                      .bind("body")))
          .bind("ctor"),
      this);
}

void DynamicTypeInConstructorCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor");
  const auto *Call = Result.Nodes.getNodeAs<CallExpr>("call");
  if (!Ctor || !Call)
    return;

  diag(Call->getExprLoc(),
       "dynamic type operations (typeid/dynamic_cast) should not be used in "
       "constructor");
}

} // namespace clang::tidy::hsc
