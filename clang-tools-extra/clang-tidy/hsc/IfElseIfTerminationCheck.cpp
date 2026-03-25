#include "IfElseIfTerminationCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void IfElseIfTerminationCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(ifStmt(hasElse(ifStmt())).bind("if_else_if"), this);
}

void IfElseIfTerminationCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *FirstIf = Result.Nodes.getNodeAs<clang::IfStmt>("if_else_if");
  if (!FirstIf)
    return;

  const clang::IfStmt *Current = FirstIf;
  while (Current && Current->getElse()) {
    const auto *Else = Current->getElse();
    if (const auto *NextIf = dyn_cast<clang::IfStmt>(Else)) {
      Current = NextIf;
      continue;
    }
    return;
  }

  if (Current && !Current->getElse()) {
    diag(FirstIf->getIfLoc(),
         "if-else-if chain should be terminated with an else clause");
  }
}

} // namespace clang::tidy::hsc
