#include "AdvancedMemoryManagementCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/ExprCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void AdvancedMemoryManagementCheck::registerMatchers(MatchFinder *Finder) {
  // Match all new expressions (we'll filter placement in check)
  Finder->addMatcher(cxxNewExpr().bind("newExpr"), this);

  // Explicit destructor calls
  Finder->addMatcher(
      cxxMemberCallExpr(callee(cxxDestructorDecl())).bind("explicitDtor"),
      this);

  // User-declared operator new/delete (free functions)
  Finder->addMatcher(functionDecl(hasAnyOverloadedOperatorName(
                                      "new", "new[]", "delete", "delete[]"))
                         .bind("opNewDelete"),
                     this);
}

void AdvancedMemoryManagementCheck::check(
    const MatchFinder::MatchResult &Result) {
  if (const auto *NewExpr = Result.Nodes.getNodeAs<CXXNewExpr>("newExpr")) {
    // Placement new → has placement args
    if (NewExpr->getNumPlacementArgs() > 0) {
      // Allow std::nothrow case
      bool IsNoThrow = false;

      for (unsigned i = 0; i < NewExpr->getNumPlacementArgs(); ++i) {
        const Expr *Arg = NewExpr->getPlacementArg(i)->IgnoreParenImpCasts();

        if (const auto *DRE = dyn_cast<DeclRefExpr>(Arg)) {
          if (const auto *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
            if (VD->getName() == "nothrow") {
              IsNoThrow = true;
              break;
            }
          }
        }
      }

      if (!IsNoThrow) {
        diag(NewExpr->getExprLoc(),
             "avoid placement new; advanced memory management is prohibited");
      }
    }
    return;
  }

  if (const auto *Call =
          Result.Nodes.getNodeAs<CXXMemberCallExpr>("explicitDtor")) {
    diag(Call->getExprLoc(), "explicit destructor call is prohibited");
    return;
  }

  if (const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("opNewDelete")) {
    // Only flag if it's actually a user-defined overload
    if (Func->isOverloadedOperator()) {
      diag(Func->getLocation(),
           "user-declared operator new/delete is prohibited");
    }
    return;
  }
}

} // namespace clang::tidy::hsc
