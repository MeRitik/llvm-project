#include "VirtualOverrideCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void VirtualOverrideCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxMethodDecl(unless(isImplicit())).bind("method"), this);
}

void VirtualOverrideCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Method = Result.Nodes.getNodeAs<CXXMethodDecl>("method");
  if (!Method || !Method->isFirstDecl())
    return;

  const bool HasVirtual = Method->isVirtualAsWritten();
  const bool HasOverride = Method->hasAttr<clang::OverrideAttr>();
  const bool HasFinal = Method->hasAttr<clang::FinalAttr>();
  const bool OverridesBase = Method->size_overridden_methods() > 0;

  if (HasVirtual && HasOverride) {
    diag(Method->getLocation(),
         "use only one specifier: avoid combining 'virtual' and 'override'");
  }

  if (HasOverride && HasFinal) {
    diag(Method->getLocation(),
         "use only one specifier: avoid combining 'override' and 'final'");
  }

  if (HasVirtual && HasFinal) {
    diag(Method->getLocation(),
         "use only one specifier: avoid combining 'virtual' and 'final'");
  }

  if (OverridesBase && !HasOverride && !HasFinal) {
    diag(Method->getLocation(),
         "overriding virtual member function should use 'override' or "
         "'final'");
  }
}

} // namespace clang::tidy::hsc
