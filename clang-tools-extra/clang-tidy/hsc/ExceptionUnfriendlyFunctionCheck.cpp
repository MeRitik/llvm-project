#include "ExceptionUnfriendlyFunctionCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void ExceptionUnfriendlyFunctionCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxDestructorDecl(unless(isNoThrow())).bind("dtor"), this);
  Finder->addMatcher(
      cxxConstructorDecl(isMoveConstructor(), unless(isNoThrow()))
          .bind("movecon"),
      this);
  Finder->addMatcher(
      cxxMethodDecl(isMoveAssignmentOperator(), unless(isNoThrow()))
          .bind("moveassign"),
      this);
}

void ExceptionUnfriendlyFunctionCheck::check(
    const MatchFinder::MatchResult &Result) {
  if (const auto *Dtor = Result.Nodes.getNodeAs<CXXDestructorDecl>("dtor")) {
    diag(Dtor->getLocation(), "destructors should be declared 'noexcept'");
    return;
  }

  if (const auto *MoveCon =
          Result.Nodes.getNodeAs<CXXConstructorDecl>("movecon")) {
    diag(MoveCon->getLocation(),
         "move constructors should be declared 'noexcept'");
    return;
  }

  if (const auto *MoveAssign =
          Result.Nodes.getNodeAs<CXXMethodDecl>("moveassign")) {
    diag(MoveAssign->getLocation(),
         "move assignment operators should be declared 'noexcept'");
    return;
  }
}

} // namespace clang::tidy::hsc
