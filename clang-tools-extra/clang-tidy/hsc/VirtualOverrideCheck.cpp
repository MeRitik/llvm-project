#include "VirtualOverrideCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void VirtualOverrideCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxMethodDecl(isVirtual(), hasParent(cxxRecordDecl())).bind("method"),
      this);
}

void VirtualOverrideCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Method = Result.Nodes.getNodeAs<CXXMethodDecl>("method");
  if (!Method)
    return;

  const auto *RD = dyn_cast<CXXRecordDecl>(Method->getDeclContext());
  if (!RD || RD->getNumBases() == 0)
    return;

  bool OverridesBase = false;
  for (const auto &Base : RD->bases()) {
    const auto *BaseClass = Base.getType()->getAsCXXRecordDecl();
    if (!BaseClass)
      continue;

    for (const auto *BaseMethod : BaseClass->methods()) {
      if (BaseMethod->getName() == Method->getName()) {
        OverridesBase = true;
        break;
      }
    }
    if (OverridesBase)
      break;
  }

  if (OverridesBase) {
    if (!Method->hasAttr<clang::OverrideAttr>() &&
        !Method->hasAttr<clang::FinalAttr>()) {
      diag(Method->getLocation(),
           "virtual member function should use 'override' or 'final' "
           "specifier");
    }
  }
}

} // namespace clang::tidy::hsc
