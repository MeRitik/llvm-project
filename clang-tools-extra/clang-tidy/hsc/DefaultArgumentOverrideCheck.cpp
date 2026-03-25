#include "DefaultArgumentOverrideCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void DefaultArgumentOverrideCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxMethodDecl(isVirtual(), hasAnyParameter(parmVarDecl(
                                                    hasDefaultArgument())))
                         .bind("override"),
                     this);
}

void DefaultArgumentOverrideCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *Override = Result.Nodes.getNodeAs<CXXMethodDecl>("override");
  if (!Override)
    return;

  const auto *RD = dyn_cast<CXXRecordDecl>(Override->getDeclContext());
  if (!RD || RD->getNumBases() == 0)
    return;

  for (const auto &Base : RD->bases()) {
    const auto *BaseClass = Base.getType()->getAsCXXRecordDecl();
    if (!BaseClass)
      continue;

    for (const auto *BaseMethod : BaseClass->methods()) {
      if (BaseMethod->getName() != Override->getName())
        continue;

      unsigned MinParams =
          std::min(BaseMethod->param_size(), Override->param_size());
      for (unsigned i = 0; i < MinParams; ++i) {
        const auto *BaseParam = BaseMethod->getParamDecl(i);
        const auto *OverrideParam = Override->getParamDecl(i);

        bool BaseHasDefault = BaseParam->hasDefaultArg();
        bool OverrideHasDefault = OverrideParam->hasDefaultArg();

        if (BaseHasDefault != OverrideHasDefault) {
          diag(OverrideParam->getLocation(),
               "default argument in override differs from base class "
               "definition");
        }
      }
    }
  }
}

} // namespace clang::tidy::hsc
