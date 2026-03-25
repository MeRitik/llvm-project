#include "OverlappingCopyCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void OverlappingCopyCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      callExpr(callee(functionDecl(hasName("memcpy")))).bind("memcpy"), this);
}

void OverlappingCopyCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Memcpy = Result.Nodes.getNodeAs<CallExpr>("memcpy");
  if (!Memcpy || Memcpy->getNumArgs() < 3)
    return;

  diag(
      Memcpy->getExprLoc(),
      "memcpy with potentially overlapping regions should use memmove instead");
}

} // namespace clang::tidy::hsc
