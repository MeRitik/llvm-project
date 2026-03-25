#include "DynamicMemoryManagementCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void DynamicMemoryManagementCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxNewExpr().bind("new"), this);
  Finder->addMatcher(cxxDeleteExpr().bind("del"), this);
  Finder->addMatcher(callExpr(callee(functionDecl(hasAnyName(
                                  "malloc", "free", "calloc", "realloc"))))
                         .bind("malloc"),
                     this);
}

void DynamicMemoryManagementCheck::check(
    const MatchFinder::MatchResult &Result) {
  if (const auto *New = Result.Nodes.getNodeAs<CXXNewExpr>("new")) {
    diag(New->getExprLoc(),
         "use std::make_unique or std::make_shared instead of raw new");
    return;
  }

  if (const auto *Del = Result.Nodes.getNodeAs<CXXDeleteExpr>("del")) {
    diag(Del->getExprLoc(), "use smart pointers instead of raw delete");
    return;
  }

  if (const auto *Malloc = Result.Nodes.getNodeAs<CallExpr>("malloc")) {
    diag(Malloc->getExprLoc(),
         "use std::unique_ptr or std::shared_ptr instead of malloc/free");
    return;
  }
}

} // namespace clang::tidy::hsc
