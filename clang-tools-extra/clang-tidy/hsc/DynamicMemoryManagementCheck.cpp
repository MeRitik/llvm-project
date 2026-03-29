#include "DynamicMemoryManagementCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void DynamicMemoryManagementCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxNewExpr().bind("new"), this);

  Finder->addMatcher(cxxDeleteExpr().bind("delete"), this);

  Finder->addMatcher(
      callExpr(callee(functionDecl(hasAnyName("malloc", "calloc", "realloc",
                                              "free", "aligned_alloc"))))
          .bind("c_alloc"),
      this);

  Finder->addMatcher(
      cxxMemberCallExpr(callee(cxxMethodDecl(hasName("release"))))
          .bind("release"),
      this);

  Finder->addMatcher(cxxMemberCallExpr(callee(cxxMethodDecl(
                                           hasAnyName("allocate", "deallocate"),
                                           ofClass(hasName("std::allocator")))))
                         .bind("allocator"),
                     this);
}

void DynamicMemoryManagementCheck::check(
    const MatchFinder::MatchResult &Result) {
  if (const auto *E = Result.Nodes.getNodeAs<CXXNewExpr>("new")) {
    diag(E->getBeginLoc(), "avoid raw 'new'; use automatic memory management "
                           "(e.g., std::make_unique)");
    return;
  }

  if (const auto *E = Result.Nodes.getNodeAs<CXXDeleteExpr>("delete")) {
    diag(E->getBeginLoc(),
         "avoid raw 'delete'; use automatic memory management");
    return;
  }

  if (const auto *E = Result.Nodes.getNodeAs<CallExpr>("c_alloc")) {
    diag(E->getBeginLoc(), "avoid C-style memory allocation; use RAII (e.g., "
                           "std::vector, std::make_unique)");
    return;
  }

  if (const auto *E = Result.Nodes.getNodeAs<CXXMemberCallExpr>("release")) {
    diag(E->getBeginLoc(), "avoid std::unique_ptr::release(); it breaks "
                           "automatic memory management");
    return;
  }

  if (const auto *E = Result.Nodes.getNodeAs<CXXMemberCallExpr>("allocator")) {
    diag(E->getBeginLoc(), "avoid std::allocator::allocate/deallocate; prefer "
                           "automatic containers");
    return;
  }
}

} // namespace clang::tidy::hsc
