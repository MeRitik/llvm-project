#include "MixedVirtualInheritanceCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void MixedVirtualInheritanceCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxRecordDecl(isDerivedFrom(cxxRecordDecl())).bind("record"), this);
}

void MixedVirtualInheritanceCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *RD = Result.Nodes.getNodeAs<CXXRecordDecl>("record");
  if (!RD)
    return;

  std::map<const CXXRecordDecl *, unsigned> BaseVirtuality;
  bool HasVirtual = false;
  bool HasNonVirtual = false;

  for (const auto &Base : RD->bases()) {
    const auto *BaseClass = Base.getType()->getAsCXXRecordDecl();
    if (!BaseClass)
      continue;

    if (Base.isVirtual()) {
      HasVirtual = true;
      BaseVirtuality[BaseClass] = 1;
    } else {
      HasNonVirtual = true;
      BaseVirtuality[BaseClass] = 0;
    }
  }

  if (HasVirtual && HasNonVirtual) {
    diag(RD->getLocation(),
         "mixing virtual and non-virtual inheritance of the same base class");
  }
}

} // namespace clang::tidy::hsc
