#include "ExceptionUnfriendlyFunctionCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/ExprCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

static bool isNoexcept(const FunctionDecl *FD) {
  if (!FD)
    return false;

  const auto *FPT = FD->getType()->getAs<FunctionProtoType>();
  if (!FPT)
    return false;

  // Treat dependent noexcept specifications as compliant to avoid noisy
  // diagnostics on templates that require instantiation to compute canThrow().
  return FPT->isNothrow(/*ResultIfDependent=*/true);
}

static const Expr *stripInit(const Expr *E) {
  E = E ? E->IgnoreParenImpCasts() : nullptr;
  while (E) {
    if (const auto *Cleanups = dyn_cast<ExprWithCleanups>(E)) {
      E = Cleanups->getSubExpr();
      E = E ? E->IgnoreParenImpCasts() : nullptr;
      continue;
    }
    if (const auto *BindTmp = dyn_cast<CXXBindTemporaryExpr>(E)) {
      E = BindTmp->getSubExpr();
      E = E ? E->IgnoreParenImpCasts() : nullptr;
      continue;
    }
    if (const auto *MatTmp = dyn_cast<MaterializeTemporaryExpr>(E)) {
      E = MatTmp->getSubExpr();
      E = E ? E->IgnoreParenImpCasts() : nullptr;
      continue;
    }
    break;
  }
  return E;
}

void ExceptionUnfriendlyFunctionCheck::registerMatchers(MatchFinder *Finder) {
  // Destructor
  Finder->addMatcher(cxxDestructorDecl().bind("dtor"), this);

  // Move constructor
  Finder->addMatcher(cxxConstructorDecl(isMoveConstructor()).bind("movecon"),
                     this);

  // Move assignment
  Finder->addMatcher(
      cxxMethodDecl(isMoveAssignmentOperator()).bind("moveassign"), this);

  // swap
  Finder->addMatcher(functionDecl(hasName("swap")).bind("swap"), this);

  // Global/static object
  Finder->addMatcher(
      varDecl(hasGlobalStorage(), hasInitializer(expr())).bind("globalVar"),
      this);
}

void ExceptionUnfriendlyFunctionCheck::check(
    const MatchFinder::MatchResult &Result) {
  // Destructor
  if (const auto *Dtor = Result.Nodes.getNodeAs<CXXDestructorDecl>("dtor")) {
    if (!isNoexcept(Dtor))
      diag(Dtor->getLocation(), "destructor should be declared 'noexcept'");
    return;
  }

  // Move constructor
  if (const auto *MoveCon =
          Result.Nodes.getNodeAs<CXXConstructorDecl>("movecon")) {
    if (!isNoexcept(MoveCon)) {
      diag(MoveCon->getLocation(),
           "move constructor should be declared 'noexcept'");
    }
    return;
  }

  // Move assignment
  if (const auto *MoveAssign =
          Result.Nodes.getNodeAs<CXXMethodDecl>("moveassign")) {
    if (!isNoexcept(MoveAssign)) {
      diag(MoveAssign->getLocation(),
           "move assignment operator should be declared 'noexcept'");
    }
    return;
  }

  // swap
  if (const auto *Swap = Result.Nodes.getNodeAs<FunctionDecl>("swap")) {
    if (!isNoexcept(Swap)) {
      diag(Swap->getLocation(),
           "function 'swap' should be declared 'noexcept'");
    }
    return;
  }

  // Global/static object
  if (const auto *VD = Result.Nodes.getNodeAs<VarDecl>("globalVar")) {
    if (VD->isLocalVarDecl())
      return;

    if (VD->isConstexpr())
      return;

    const Expr *Init = stripInit(VD->getInit());
    if (!Init)
      return;

    // Check the constructor directly used for initialization.
    if (const auto *CtorExpr = dyn_cast<CXXConstructExpr>(Init)) {
      const auto *Ctor = CtorExpr->getConstructor();
      if (Ctor && !isNoexcept(Ctor)) {
        diag(VD->getLocation(),
             "non-local object of type '%0' should use a noexcept constructor")
            << VD->getType().getUnqualifiedType().getAsString();
      }
      return;
    }

    // If initialization is a direct call, require the called function to be
    // non-throwing.
    if (const auto *Call = dyn_cast<CallExpr>(Init)) {
      if (const FunctionDecl *Callee = Call->getDirectCallee()) {
        if (!isNoexcept(Callee))
          diag(VD->getLocation(),
               "function used to initialize a non-local object should be "
               "declared 'noexcept'");
      }
      return;
    }
  }
}

} // namespace clang::tidy::hsc
