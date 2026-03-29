#include "ExplicitSingleArgumentConstructorCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void ExplicitSingleArgumentConstructorCheck::registerMatchers(
    MatchFinder *Finder) {
  Finder->addMatcher(cxxConstructorDecl(unless(anyOf(isImplicit(), isDeleted(),
                                                     isCopyConstructor(),
                                                     isMoveConstructor())))
                         .bind("ctor"),
                     this);

  Finder->addMatcher(cxxConversionDecl(unless(anyOf(isImplicit(), isDeleted())))
                         .bind("conversion"),
                     this);
}

void ExplicitSingleArgumentConstructorCheck::check(
    const MatchFinder::MatchResult &Result) {
  if (const auto *Conversion =
          Result.Nodes.getNodeAs<CXXConversionDecl>("conversion")) {
    if (!Conversion->isFirstDecl())
      return;

    if (Conversion->isExplicit())
      return;

    diag(Conversion->getLocation(),
         "conversion operators should be declared explicit");
    return;
  }

  const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor");
  if (!Ctor)
    return;

  if (!Ctor->isFirstDecl())
    return;

  if (Ctor->getNumParams() == 0 || Ctor->getMinRequiredArguments() > 1)
    return;

  if (Ctor->isExplicit() || Ctor->isExplicitlyDefaulted())
    return;

  diag(Ctor->getLocation(),
       "constructors callable with a single argument should be declared "
       "explicit");
}

} // namespace clang::tidy::hsc
