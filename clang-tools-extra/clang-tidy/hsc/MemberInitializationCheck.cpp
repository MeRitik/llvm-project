#include "MemberInitializationCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void MemberInitializationCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxConstructorDecl(isDefinition(),
                         hasAnyParameter(parmVarDecl().bind("param")))
          .bind("ctor"),
      this);
}

void MemberInitializationCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor");
  if (!Ctor)
    return;

  const auto *RD = Ctor->getParent();
  if (!RD)
    return;

  for (const auto *Field : RD->fields()) {
    if (Field->hasInClassInitializer())
      continue;

    bool Found = false;
    for (const auto *Init : Ctor->inits()) {
      if (Init->getMember() == Field) {
        Found = true;
        break;
      }
    }

    if (!Found && !Field->getType().isConstQualified()) {
      diag(Ctor->getLocation(),
           "data member '%0' is not explicitly initialized")
          << Field->getName();
    }
  }
}

} // namespace clang::tidy::hsc
