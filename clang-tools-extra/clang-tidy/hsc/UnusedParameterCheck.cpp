#include "UnusedParameterCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

AST_MATCHER(ParmVarDecl, isNamedParameter) { return !Node.getName().empty(); }

void UnusedParameterCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      functionDecl(
          hasAnyParameter(parmVarDecl(isNamedParameter()).bind("param")))
          .bind("func"),
      this);
}

void UnusedParameterCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Param = Result.Nodes.getNodeAs<ParmVarDecl>("param");
  if (!Param || Param->isReferenced() || Param->hasAttr<clang::UnusedAttr>())
    return;
  if (Param->getName().empty())
    return;

  diag(Param->getLocation(), "unused named parameter '%0' detected")
      << Param->getName();
}

} // namespace clang::tidy::hsc
