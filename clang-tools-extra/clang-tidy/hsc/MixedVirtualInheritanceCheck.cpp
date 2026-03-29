#include "MixedVirtualInheritanceCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

enum BaseVirtualityFlags : unsigned char {
  SeenVirtual = 1,
  SeenNonVirtual = 2,
};

void collectBaseVirtuality(
    const CXXRecordDecl *RD,
    llvm::DenseMap<const CXXRecordDecl *, unsigned char> &VirtualityByBase) {
  const auto *Definition = RD->getDefinition();
  if (!Definition)
    return;

  llvm::SmallVector<const CXXRecordDecl *, 8> Worklist;
  llvm::DenseSet<const CXXRecordDecl *> Visited;
  Worklist.push_back(Definition);

  while (!Worklist.empty()) {
    const auto *Current = Worklist.pop_back_val();
    if (!Visited.insert(Current).second)
      continue;

    for (const auto &Base : Current->bases()) {
      const auto *BaseClass = Base.getType()->getAsCXXRecordDecl();
      if (!BaseClass)
        continue;

      const auto *BaseDefinition = BaseClass->getDefinition();
      const auto *CanonicalBase =
          (BaseDefinition ? BaseDefinition : BaseClass)->getCanonicalDecl();

      unsigned char &Flags = VirtualityByBase[CanonicalBase];
      Flags |= Base.isVirtual() ? SeenVirtual : SeenNonVirtual;

      if (BaseDefinition)
        Worklist.push_back(BaseDefinition);
    }
  }
}

} // namespace

void MixedVirtualInheritanceCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxRecordDecl(isDerivedFrom(cxxRecordDecl())).bind("record"), this);
}

void MixedVirtualInheritanceCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *RD = Result.Nodes.getNodeAs<CXXRecordDecl>("record");
  if (!RD || !RD->hasDefinition())
    return;

  llvm::DenseMap<const CXXRecordDecl *, unsigned char> VirtualityByBase;
  collectBaseVirtuality(RD, VirtualityByBase);

  for (const auto &Entry : VirtualityByBase) {
    if ((Entry.second & (SeenVirtual | SeenNonVirtual)) !=
        (SeenVirtual | SeenNonVirtual))
      continue;

    diag(RD->getLocation(),
         "base class %0 is inherited both virtually and non-virtually in "
         "this hierarchy")
        << Entry.first;
  }
}

} // namespace clang::tidy::hsc
