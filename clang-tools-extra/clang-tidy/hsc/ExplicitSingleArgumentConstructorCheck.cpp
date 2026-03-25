#include "ExplicitSingleArgumentConstructorCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void ExplicitSingleArgumentConstructorCheck::registerMatchers(
    MatchFinder *Finder) {
  Finder->addMatcher(
      cxxConstructorDecl(
          unless(isDeleted()), parameterCountIs(1), unless(isCopyConstructor()),
          unless(isMoveConstructor()), hasParent(cxxRecordDecl()))
          .bind("ctor"),
      this);
}

void ExplicitSingleArgumentConstructorCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor");
  if (!Ctor)
    return;

  if (Ctor->isExplicit() || Ctor->isExplicitlyDefaulted())
    return;

  diag(Ctor->getLocation(),
       "single-argument constructors should be declared explicit");
}

} // namespace clang::tidy::hsc
