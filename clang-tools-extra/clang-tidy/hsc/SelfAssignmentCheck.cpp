#include "SelfAssignmentCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void SelfAssignmentCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxMethodDecl(
          hasName("operator="),
          hasParameter(0, parmVarDecl(hasType(references(cxxRecordDecl())))),
          isDefinition(), hasBody(compoundStmt()))
          .bind("op"),
      this);
}

void SelfAssignmentCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Op = Result.Nodes.getNodeAs<CXXMethodDecl>("op");
  if (!Op || Op->param_size() != 1)
    return;

  const auto *Body = Op->getBody();
  const auto *Compound = dyn_cast<CompoundStmt>(Body);
  if (!Compound)
    return;

  bool HasSelfCheck = false;
  for (const auto *Stmt : Compound->body()) {
    const auto *IfCondition = dyn_cast<clang::IfStmt>(Stmt);
    if (!IfCondition)
      continue;

    const auto *Cond = IfCondition->getCond();
    const auto *BinOp = dyn_cast<BinaryOperator>(Cond);
    if (!BinOp || BinOp->getOpcode() != BO_NE)
      continue;

    HasSelfCheck = true;
    break;
  }

  if (!HasSelfCheck) {
    diag(Op->getLocation(),
         "assignment operator should check for self-assignment");
  }
}

} // namespace clang::tidy::hsc
