#include "ReinterpretCastCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void ReinterpretCastCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxReinterpretCastExpr().bind("reinterpret_cast"), this);
}

void ReinterpretCastCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *ReinterpretCast =
      Result.Nodes.getNodeAs<CXXReinterpretCastExpr>("reinterpret_cast");
  if (!ReinterpretCast)
    return;

  diag(ReinterpretCast->getExprLoc(),
       "reinterpret_cast should be used with extreme caution; prefer "
       "safer alternatives");
}

} // namespace clang::tidy::hsc
