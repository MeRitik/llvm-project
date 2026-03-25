#include "ExceptionPointerCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void ExceptionPointerCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxThrowExpr().bind("throw"), this);
}

static bool isPointerType(const Expr *E) {
  const QualType T = E->getType();
  return T->isPointerType();
}

void ExceptionPointerCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Throw = Result.Nodes.getNodeAs<CXXThrowExpr>("throw");
  if (!Throw || !Throw->getSubExpr())
    return;

  const auto *Arg = Throw->getSubExpr();
  if (!Arg)
    return;

  if (isPointerType(Arg)) {
    diag(Throw->getThrowLoc(),
         "exception should not be thrown as pointer; throw by value instead");
  }
}

} // namespace clang::tidy::hsc
