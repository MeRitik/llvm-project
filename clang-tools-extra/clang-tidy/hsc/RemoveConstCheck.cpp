#include "RemoveConstCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void RemoveConstCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxConstCastExpr().bind("const_cast"), this);
}

void RemoveConstCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *ConstCast =
      Result.Nodes.getNodeAs<CXXConstCastExpr>("const_cast");
  if (!ConstCast)
    return;

  diag(ConstCast->getExprLoc(),
       "const_cast should not be used to remove const qualifier");
}

} // namespace clang::tidy::hsc
