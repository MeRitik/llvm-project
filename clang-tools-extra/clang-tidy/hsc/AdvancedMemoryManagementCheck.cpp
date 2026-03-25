#include "AdvancedMemoryManagementCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AdvancedMemoryManagementCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      callExpr(callee(functionDecl(hasName("operator new")))).bind("op_new"),
      this);
  Finder->addMatcher(callExpr(callee(functionDecl(hasName("operator delete"))))
                         .bind("op_delete"),
                     this);
  Finder->addMatcher(cxxNewExpr().bind("new_expr"), this);
}

void AdvancedMemoryManagementCheck::check(
    const MatchFinder::MatchResult &Result) {
  if (const auto *NewExpr = Result.Nodes.getNodeAs<CXXNewExpr>("new_expr")) {
    if (NewExpr->getNumPlacementArgs() > 0)
      diag(NewExpr->getExprLoc(), "placement new should be avoided");
    return;
  }

  if (const auto *OpNew = Result.Nodes.getNodeAs<CallExpr>("op_new")) {
    diag(OpNew->getExprLoc(),
         "user-declared operator new/delete should be avoided");
    return;
  }

  if (const auto *OpDelete = Result.Nodes.getNodeAs<CallExpr>("op_delete")) {
    diag(OpDelete->getExprLoc(),
         "user-declared operator new/delete should be avoided");
    return;
  }

  if (const auto *PlacementNew =
          Result.Nodes.getNodeAs<CXXNewExpr>("placement_new")) {
    diag(PlacementNew->getExprLoc(),
         "placement new should be used with caution");
    return;
  }
}

} // namespace clang::tidy::hsc
