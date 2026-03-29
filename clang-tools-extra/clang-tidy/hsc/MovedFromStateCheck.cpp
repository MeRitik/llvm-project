#include "MovedFromStateCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

class DeclRefCollector : public RecursiveASTVisitor<DeclRefCollector> {
public:
  explicit DeclRefCollector(const VarDecl *Var) : Var(Var) {}

  bool VisitDeclRefExpr(DeclRefExpr *DRE) {
    if (DRE && DRE->getDecl() == Var)
      Refs.push_back(DRE);
    return true;
  }

  llvm::SmallVector<const DeclRefExpr *, 4> Refs;

private:
  const VarDecl *Var;
};

static bool isStdUniquePtr(QualType Type) {
  const auto *TST = Type->getAs<TemplateSpecializationType>();
  if (!TST)
    return false;

  const TemplateDecl *TD = TST->getTemplateName().getAsTemplateDecl();
  if (!TD)
    return false;

  return TD->getQualifiedNameAsString() == "std::unique_ptr";
}

static bool isOnLHSOfAssignment(const DeclRefExpr *DRE, ASTContext &Context) {
  if (!DRE)
    return false;

  const Expr *Current = DRE;
  while (Current) {
    auto Parents = Context.getParents(*Current);
    if (Parents.empty())
      return false;

    bool Advanced = false;
    for (const auto &Parent : Parents) {
      if (const auto *BO = Parent.get<BinaryOperator>()) {
        if (BO->isAssignmentOp() && BO->getLHS()->IgnoreParenImpCasts() ==
                                        Current->IgnoreParenImpCasts())
          return true;
        continue;
      }

      if (const auto *OpCall = Parent.get<CXXOperatorCallExpr>()) {
        if (OpCall->getOperator() == OO_Equal && OpCall->getNumArgs() > 0 &&
            OpCall->getArg(0)->IgnoreParenImpCasts() ==
                Current->IgnoreParenImpCasts())
          return true;
        continue;
      }

      if (const auto *ParentExpr = Parent.get<Expr>()) {
        Current = ParentExpr;
        Advanced = true;
        break;
      }
    }

    if (!Advanced)
      return false;
  }

  return false;
}

static const Stmt *getEnclosingTopLevelStmt(const Stmt *S,
                                            const CompoundStmt *Body,
                                            ASTContext &Context) {
  if (!S || !Body)
    return nullptr;

  DynTypedNode Node = DynTypedNode::create(*S);
  const Stmt *LastStmt = S;

  while (true) {
    auto Parents = Context.getParents(Node);
    if (Parents.empty())
      return LastStmt;

    const DynTypedNode &Parent = *Parents.begin();
    if (const auto *ParentStmt = Parent.get<Stmt>()) {
      if (ParentStmt == Body)
        return LastStmt;
      LastStmt = ParentStmt;
      Node = Parent;
      continue;
    }

    if (Parent.get<Decl>()) {
      Node = Parent;
      continue;
    }

    return LastStmt;
  }
}

} // namespace

void MovedFromStateCheck::registerMatchers(MatchFinder *Finder) {
  if (!getLangOpts().CPlusPlus)
    return;

  Finder->addMatcher(
      callExpr(callee(functionDecl(matchesName("^(::)?std::(move|forward)$"))),
               hasArgument(0, ignoringParenImpCasts(
                                  declRefExpr(to(varDecl().bind("movedVar"))))),
               hasAncestor(functionDecl().bind("func")))
          .bind("moveCall"),
      this);
}

void MovedFromStateCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Call = Result.Nodes.getNodeAs<CallExpr>("moveCall");
  const auto *Var = Result.Nodes.getNodeAs<VarDecl>("movedVar");
  const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("func");

  if (!Call || !Var || !Func || !Func->hasBody())
    return;

  // Skip std::unique_ptr: moved-from state is well-defined (nullptr).
  if (isStdUniquePtr(Var->getType()))
    return;

  const auto *Body = dyn_cast<CompoundStmt>(Func->getBody());
  if (!Body)
    return;

  const Stmt *MoveStmt = getEnclosingTopLevelStmt(Call, Body, *Result.Context);
  if (!MoveStmt)
    return;

  bool AfterMove = false;

  for (const Stmt *S : Body->body()) {
    if (!S)
      continue;

    if (S == MoveStmt) {
      AfterMove = true;
      continue;
    }

    if (!AfterMove)
      continue;

    DeclRefCollector Collector(Var);
    Collector.TraverseStmt(const_cast<Stmt *>(S));
    if (Collector.Refs.empty())
      continue;

    const bool IsReset =
        llvm::all_of(Collector.Refs, [&](const DeclRefExpr *DRE) {
          return isOnLHSOfAssignment(DRE, *Result.Context);
        });
    if (IsReset)
      return;

    const SourceManager &SM = *Result.SourceManager;
    const DeclRefExpr *FirstUse = *llvm::min_element(
        Collector.Refs, [&](const DeclRefExpr *A, const DeclRefExpr *B) {
          return SM.isBeforeInTranslationUnit(A->getExprLoc(), B->getExprLoc());
        });

    diag(FirstUse->getExprLoc(), "object used after being moved-from");
    return;
  }
}

} // namespace clang::tidy::hsc
