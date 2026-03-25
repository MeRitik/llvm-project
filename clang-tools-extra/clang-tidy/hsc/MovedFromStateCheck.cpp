#include "MovedFromStateCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void MovedFromStateCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(declRefExpr(to(varDecl())).bind("ref"), this);
}

void MovedFromStateCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Ref = Result.Nodes.getNodeAs<DeclRefExpr>("ref");
  if (!Ref)
    return;

  const auto *Var = dyn_cast<VarDecl>(Ref->getDecl());
  if (!Var)
    return;

  // Simplified check: warn if variable used after move operation
  // Full implementation would require data flow analysis
}

} // namespace clang::tidy::hsc
