#include "SelfAssignmentCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void SelfAssignmentCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxMethodDecl(isDefinition(), isCopyAssignmentOperator(),
                                   hasParameter(0, parmVarDecl().bind("param")),
                                   hasBody(stmt().bind("body")))
                         .bind("op"),
                     this);
}

// Detect: &other OR std::addressof(other)
static bool isAddressOfParam(const Expr *E, const ParmVarDecl *Param) {
  E = E->IgnoreParenImpCasts();

  // Case 1: &other
  if (const auto *UO = dyn_cast<UnaryOperator>(E)) {
    if (UO->getOpcode() == UO_AddrOf) {
      if (const auto *DRE =
              dyn_cast<DeclRefExpr>(UO->getSubExpr()->IgnoreParenImpCasts())) {
        return DRE->getDecl() == Param;
      }
    }
  }

  // Case 2: std::addressof(other)
  if (const auto *Call = dyn_cast<CallExpr>(E)) {
    if (Call->getNumArgs() == 1) {
      if (const auto *Arg =
              dyn_cast<DeclRefExpr>(Call->getArg(0)->IgnoreParenImpCasts())) {
        if (Arg->getDecl() == Param)
          return true;
      }
    }
  }

  return false;
}

// Detect: this == &other OR this != &other
static bool isSelfCheckCondition(const Expr *Cond, const ParmVarDecl *Param) {
  Cond = Cond->IgnoreParenImpCasts();

  const auto *BinOp = dyn_cast<BinaryOperator>(Cond);
  if (!BinOp)
    return false;

  if (BinOp->getOpcode() != BO_EQ && BinOp->getOpcode() != BO_NE)
    return false;

  const Expr *LHS = BinOp->getLHS()->IgnoreParenImpCasts();
  const Expr *RHS = BinOp->getRHS()->IgnoreParenImpCasts();

  const auto *ThisL = dyn_cast<CXXThisExpr>(LHS);
  const auto *ThisR = dyn_cast<CXXThisExpr>(RHS);

  if ((ThisL && isAddressOfParam(RHS, Param)) ||
      (ThisR && isAddressOfParam(LHS, Param))) {
    return true;
  }

  return false;
}

// Recursively search for self-check
static bool hasSelfAssignmentCheck(const Stmt *S, const ParmVarDecl *Param) {
  if (!S)
    return false;

  if (const auto *If = dyn_cast<IfStmt>(S)) {
    if (isSelfCheckCondition(If->getCond(), Param))
      return true;
  }

  for (const Stmt *Child : S->children())
    if (hasSelfAssignmentCheck(Child, Param))
      return true;

  return false;
}

// Detect copy-and-swap: Type temp(other);
static bool usesCopyAndSwap(const Stmt *S, const ParmVarDecl *Param) {
  if (!S)
    return false;

  if (const auto *DS = dyn_cast<DeclStmt>(S)) {
    for (const auto *D : DS->decls()) {
      if (const auto *VD = dyn_cast<VarDecl>(D)) {
        if (const auto *Ctor =
                dyn_cast_or_null<CXXConstructExpr>(VD->getInit())) {
          if (Ctor->getNumArgs() == 1) {
            if (const auto *Arg = dyn_cast<DeclRefExpr>(
                    Ctor->getArg(0)->IgnoreParenImpCasts())) {
              if (Arg->getDecl() == Param)
                return true;
            }
          }
        }
      }
    }
  }

  for (const Stmt *Child : S->children())
    if (usesCopyAndSwap(Child, Param))
      return true;

  return false;
}

void SelfAssignmentCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Op = Result.Nodes.getNodeAs<CXXMethodDecl>("op");
  const auto *Param = Result.Nodes.getNodeAs<ParmVarDecl>("param");
  const auto *Body = Result.Nodes.getNodeAs<Stmt>("body");

  if (!Op || !Param || !Body)
    return;

  // Skip move assignment
  if (Param->getType()->isRValueReferenceType())
    return;

  if (!hasSelfAssignmentCheck(Body, Param) && !usesCopyAndSwap(Body, Param)) {
    diag(Op->getLocation(),
         "assignment operator should guard against self-assignment "
         "(e.g., 'if (this != &other)')");
  }
}

} // namespace clang::tidy::hsc
