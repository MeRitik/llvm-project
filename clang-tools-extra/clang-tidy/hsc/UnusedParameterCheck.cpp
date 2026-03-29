#include "UnusedParameterCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

class ParamReferenceVisitor
    : public ::clang::RecursiveASTVisitor<ParamReferenceVisitor> {
public:
  explicit ParamReferenceVisitor(const ParmVarDecl *Target) : Target(Target) {}

  bool VisitDeclRefExpr(DeclRefExpr *Ref) {
    if (Ref->getDecl() == Target)
      FoundReference = true;
    return true;
  }

  bool hasReference() const { return FoundReference; }

private:
  const ParmVarDecl *Target;
  bool FoundReference = false;
};

static bool isParameterUsedInBody(const ParmVarDecl *Param, const Stmt *Body) {
  if (!Param || !Body)
    return false;

  ParamReferenceVisitor Visitor(Param);
  Visitor.TraverseStmt(const_cast<Stmt *>(Body));
  return Visitor.hasReference();
}

} // namespace

void UnusedParameterCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      functionDecl(isDefinition(), unless(isImplicit())).bind("func"), this);
}

void UnusedParameterCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("func");
  if (!Func || !Func->doesThisDeclarationHaveABody())
    return;

  const Stmt *Body = Func->getBody();
  if (!Body)
    return;

  for (const ParmVarDecl *Param : Func->parameters()) {
    if (!Param)
      continue;
    if (Param->getName().empty())
      continue;
    if (Param->hasAttr<clang::UnusedAttr>())
      continue;
    if (isParameterUsedInBody(Param, Body))
      continue;

    diag(Param->getLocation(), "unused named parameter '%0' detected")
        << Param->getName();
  }
}

} // namespace clang::tidy::hsc
