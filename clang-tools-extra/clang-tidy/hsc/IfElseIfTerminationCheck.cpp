#include "IfElseIfTerminationCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void IfElseIfTerminationCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      ifStmt(hasElse(ifStmt()), unless(hasParent(ifStmt(hasElse(stmt())))))
          .bind("if_else_if"),
      this);
}

void IfElseIfTerminationCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Root = Result.Nodes.getNodeAs<clang::IfStmt>("if_else_if");
  if (!Root)
    return;

  const clang::IfStmt *Current = Root;

  while (true) {
    const Stmt *Else = Current->getElse();
    if (!Else)
      break;

    if (const auto *NextIf = dyn_cast<clang::IfStmt>(Else))
      Current = NextIf;
    else
      return; // proper final else exists
  }

  diag(Root->getIfLoc(),
       "if-else-if chain should be terminated with an else clause");
}

} // namespace clang::tidy::hsc
