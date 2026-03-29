#include "OverlappingCopyCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "llvm/ADT/APSInt.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

const Expr *stripExpr(const Expr *E) {
  return E ? E->IgnoreParenImpCasts() : nullptr;
}

bool isSameBase(const Expr *A, const Expr *B) {
  if (!A || !B)
    return false;

  if (const auto *D1 = dyn_cast<DeclRefExpr>(A)) {
    if (const auto *D2 = dyn_cast<DeclRefExpr>(B)) {
      return D1->getDecl() == D2->getDecl();
    }
  }

  return false;
}

bool evalAsNonNegativeUInt64(const Expr *E, ASTContext &Context,
                            uint64_t &Value) {
  if (!E)
    return false;

  E = stripExpr(E);
  if (!E)
    return false;

  Expr::EvalResult Result;
  if (!E->EvaluateAsInt(Result, Context))
    return false;

  llvm::APSInt IntVal = Result.Val.getInt();
  if (IntVal.isNegative())
    return false;

  Value = IntVal.getZExtValue();
  return true;
}

bool getArrayBaseAndOffsetBytes(const Expr *PtrExpr, ASTContext &Context,
                                const Expr *&Base, uint64_t &OffsetBytes) {
  Base = nullptr;
  OffsetBytes = 0;

  PtrExpr = stripExpr(PtrExpr);
  if (!PtrExpr)
    return false;

  const auto *UO = dyn_cast<UnaryOperator>(PtrExpr);
  if (!UO || UO->getOpcode() != UO_AddrOf)
    return false;

  const Expr *Sub = stripExpr(UO->getSubExpr());
  if (!Sub)
    return false;

  const Expr *ArrayBase = nullptr;
  const Expr *IndexExpr = nullptr;

  if (const auto *ASE = dyn_cast<ArraySubscriptExpr>(Sub)) {
    ArrayBase = stripExpr(ASE->getBase());
    IndexExpr = ASE->getIdx();
  } else if (const auto *Call = dyn_cast<CXXOperatorCallExpr>(Sub)) {
    if (Call->getOperator() != OO_Subscript || Call->getNumArgs() < 2)
      return false;

    ArrayBase = stripExpr(Call->getArg(0));
    IndexExpr = Call->getArg(1);
  } else {
    return false;
  }

  if (!ArrayBase || !IndexExpr)
    return false;

  uint64_t Index = 0;
  if (!evalAsNonNegativeUInt64(IndexExpr, Context, Index))
    return false;

  const QualType ElemType =
      Sub->getType()->getPointeeType().isNull()
          ? Sub->getType()
          : Sub->getType()->getPointeeType();

  auto ElemSize = Context.getTypeSizeInChars(ElemType);

  Base = ArrayBase;
  OffsetBytes = Index * static_cast<uint64_t>(ElemSize.getQuantity());
  return true;
}

bool hasOverlappingConstantMemcpyRegions(const Expr *DestPtr,
                                         const Expr *SrcPtr,
                                         const Expr *SizeExpr,
                                         ASTContext &Context) {
  uint64_t SizeBytes = 0;
  if (!evalAsNonNegativeUInt64(SizeExpr, Context, SizeBytes))
    return false;

  const Expr *DestBase = nullptr;
  const Expr *SrcBase = nullptr;
  uint64_t DestOff = 0;
  uint64_t SrcOff = 0;

  if (!getArrayBaseAndOffsetBytes(DestPtr, Context, DestBase, DestOff) ||
      !getArrayBaseAndOffsetBytes(SrcPtr, Context, SrcBase, SrcOff))
    return false;

  if (!isSameBase(DestBase, SrcBase))
    return false;

  uint64_t DestBegin = DestOff;
  uint64_t DestEnd = DestOff + SizeBytes;
  uint64_t SrcBegin = SrcOff;
  uint64_t SrcEnd = SrcOff + SizeBytes;

  return DestBegin < SrcEnd && SrcBegin < DestEnd;
}

bool isUnionType(QualType T) {
  const RecordType *RT = T->getAs<RecordType>();
  return RT && RT->getDecl()->isUnion();
}

} 

void OverlappingCopyCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      callExpr(
          callee(functionDecl(hasAnyName(
              "::memcpy", "std::memcpy",
              "__builtin_memcpy", "__builtin_memcpy_chk",
              "__builtin___memcpy_chk"))),
          hasArgument(0, expr().bind("dest")),
          hasArgument(1, expr().bind("src")),
          hasArgument(2, expr().bind("size")))
          .bind("memcpy"),
      this);

  Finder->addMatcher(
      binaryOperator(
          hasOperatorName("="),
          hasLHS(memberExpr().bind("lhs")),
          hasRHS(ignoringParenImpCasts(memberExpr().bind("rhs"))))
          .bind("assign"),
      this);
}

void OverlappingCopyCheck::check(
    const MatchFinder::MatchResult &Result) {

  if (const auto *Memcpy =
          Result.Nodes.getNodeAs<CallExpr>("memcpy")) {

    const auto *Dest = Result.Nodes.getNodeAs<Expr>("dest");
    const auto *Src  = Result.Nodes.getNodeAs<Expr>("src");
    const auto *Size = Result.Nodes.getNodeAs<Expr>("size");

    if (!Memcpy || !Dest || !Src || !Size || !Result.Context)
      return;

    if (hasOverlappingConstantMemcpyRegions(Dest, Src, Size, *Result.Context))
      diag(Memcpy->getExprLoc(),
           "memcpy on overlapping or same object; use memmove instead");

    return;
  }

  if (const auto *Assign =
          Result.Nodes.getNodeAs<BinaryOperator>("assign")) {

    const auto *LHS = Result.Nodes.getNodeAs<MemberExpr>("lhs");
    const auto *RHS = Result.Nodes.getNodeAs<MemberExpr>("rhs");

    if (!LHS || !RHS)
      return;

    const Expr *LBase = stripExpr(LHS->getBase());
    const Expr *RBase = stripExpr(RHS->getBase());

    if (!LBase || !RBase)
      return;

    if (!isUnionType(LBase->getType()))
      return;

    if (!isSameBase(LBase, RBase))
      return;

    diag(Assign->getExprLoc(),
         "copy between members of the same union object is not allowed");
  }
}

} // namespace clang::tidy::hsc