#include "MemberInitializationCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

// Class-type members are default-initialized when omitted from a constructor
// initializer list, while scalar members are left indeterminate.
static bool isPotentiallyUninitializedWhenOmitted(QualType FieldType) {
  if (FieldType.isNull())
    return false;

  while (const auto *AT = FieldType->getAsArrayTypeUnsafe())
    FieldType = AT->getElementType();

  const Type *Ty = FieldType.getTypePtrOrNull();
  if (!Ty)
    return false;

  if (Ty->isDependentType())
    return false;

  if (Ty->isRecordType())
    return false;

  return true;
}

// Check if a record type has any uninitialized scalar members
static bool recordHasUninitializedScalarMembers(const CXXRecordDecl *RD) {
  if (!RD)
    return false;

  RD = RD->getDefinition();
  if (!RD)
    return false;

  for (const auto *Field : RD->fields()) {
    if (Field->isUnnamedBitField())
      continue;

    if (Field->hasInClassInitializer())
      continue;

    if (isPotentiallyUninitializedWhenOmitted(Field->getType()))
      return true;
  }

  return false;
}

// Check if a variable is zero-initialized (e.g., DataPoint dp{};)
static bool isZeroInitialized(const VarDecl *Var) {
  if (!Var)
    return false;

  if (Var->hasInit()) {
    const Expr *Init = Var->getInit()->IgnoreImplicitAsWritten();
    if (const auto *IL = dyn_cast<InitListExpr>(Init)) {
      // For '{}' the semantic form can contain implicit value-initializers.
      // Use the syntactic form to preserve what was written in source.
      if (const auto *Syn = IL->getSyntacticForm())
        return Syn->getNumInits() == 0;
      return IL->getNumInits() == 0;
    }
  }

  return false;
}

} // namespace

void MemberInitializationCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(cxxConstructorDecl(unless(anyOf(isCopyConstructor(),
                                                     isMoveConstructor())))
                         .bind("ctor"),
                     this);

  // Match variable declarations of record types to check for unsafe usage
  Finder->addMatcher(varDecl(hasType(cxxRecordDecl())).bind("var"), this);
}

void MemberInitializationCheck::check(const MatchFinder::MatchResult &Result) {
  // Check variable declarations for unsafe record usage
  if (const auto *Var = Result.Nodes.getNodeAs<VarDecl>("var")) {
    const auto *RD = Var->getType()->getAsCXXRecordDecl();
    if (recordHasUninitializedScalarMembers(RD) && !isZeroInitialized(Var)) {
      diag(Var->getLocation(),
           "variable '%0' uses aggregate type with uninitialized members; "
           "use '%0{}' to zero-initialize")
          << Var->getName();
    }
    return;
  }

  const auto *Ctor = Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor");
  if (!Ctor)
    return;

  if (Ctor->isDelegatingConstructor())
    return;

  const auto *RD = Ctor->getParent();
  if (!RD)
    return;

  // Only diagnose the first declaration of each constructor to avoid duplicates
  // (e.g., for out-of-line definitions or when there are multiple
  // declarations).
  if (!Ctor->isFirstDecl())
    return;

  for (const auto *Field : RD->fields()) {
    if (Field->isUnnamedBitField())
      continue;

    if (Field->hasInClassInitializer())
      continue;

    bool Found = false;
    for (const auto *Init : Ctor->inits()) {
      if (Init->isMemberInitializer() && Init->getMember() == Field) {
        Found = true;
        break;
      }
    }

    if (!Found && isPotentiallyUninitializedWhenOmitted(Field->getType())) {
      diag(Ctor->getLocation(),
           "data member '%0' is not explicitly initialized")
          << Field->getName();
    }
  }
}

} // namespace clang::tidy::hsc
