//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "DependentBaseLookupCheck.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"

#include <optional>

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

void DependentBaseLookupCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      callExpr(hasAncestor(cxxRecordDecl().bind("templated-derived")))
          .bind("any-call"),
      this);

  Finder->addMatcher(
      typeLoc(loc(typedefType()),
              hasAncestor(cxxRecordDecl().bind("templated-derived")))
          .bind("unqualified-type"),
      this);
}

void DependentBaseLookupCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *RD = Result.Nodes.getNodeAs<CXXRecordDecl>("templated-derived");
  if (!RD)
    return;

  if (!RD->hasAnyDependentBases())
    return;

  if (const auto *Call = Result.Nodes.getNodeAs<CallExpr>("any-call")) {
    const Expr *Callee = Call->getCallee()->IgnoreParenImpCasts();

    if (isa<MemberExpr>(Callee))
      return;

    if (const auto *DRE = dyn_cast<DeclRefExpr>(Callee)) {
      if (DRE->hasQualifier())
        return;

      const auto *FD =
          dyn_cast_or_null<FunctionDecl>(DRE->getDecl()->getUnderlyingDecl());
      if (!FD || isa<CXXMethodDecl>(FD))
        return;

      const DeclContext *DC = FD->getDeclContext();
      if (!isa<NamespaceDecl>(DC) && !isa<TranslationUnitDecl>(DC))
        return;

      diag(Call->getExprLoc(),
           "unqualified lookup in a class with dependent base resolved to %0; "
           "use qualification or this->")
          << FD;
      return;
    }

    if (const auto *ULE = dyn_cast<UnresolvedLookupExpr>(Callee)) {
      if (ULE->getQualifier())
        return;

      diag(Call->getExprLoc(),
           "unqualified lookup in a class with dependent base resolved to %0; "
           "use qualification or this->")
          << ULE->getName();
      return;
    }

    if (const auto *DSME = dyn_cast<CXXDependentScopeMemberExpr>(Callee)) {
      if (!DSME->isImplicitAccess())
        return;

      if (DSME->getQualifier())
        return;

      diag(Call->getExprLoc(),
           "unqualified lookup in a class with dependent base resolved to %0; "
           "use qualification or this->")
          << DSME->getMember();
      return;
    }
  }

  if (const auto *TL = Result.Nodes.getNodeAs<TypeLoc>("unqualified-type")) {
    const auto TTL = TL->getAs<TypedefTypeLoc>();
    if (TTL.isNull())
      return;

    const TypedefType *TT = TTL.getTypePtr();
    const TypedefNameDecl *TD = TT ? TT->getDecl() : nullptr;
    if (!TD)
      return;

    const DeclContext *DC = TD->getDeclContext();
    if (!isa<NamespaceDecl>(DC) && !isa<TranslationUnitDecl>(DC))
      return;

    // Ignore explicitly qualified type names such as ::Type or Ns::Type.
    Token CurrentTok;
    if (!Lexer::getRawToken(TL->getBeginLoc(), CurrentTok,
                            *Result.SourceManager, getLangOpts(),
                            /*IgnoreWhiteSpace=*/true) &&
        CurrentTok.is(tok::coloncolon)) {
      return;
    }

    if (const std::optional<Token> PrevTok = Lexer::findPreviousToken(
            TL->getBeginLoc(), *Result.SourceManager, getLangOpts(),
            /*IncludeComments=*/false)) {
      if (PrevTok->is(tok::coloncolon))
        return;
    }

    diag(TL->getBeginLoc(),
         "unqualified type lookup in a class with dependent base resolved to "
         "%0; use qualification")
        << TD;
  }
}

} // namespace clang::tidy::hsc
