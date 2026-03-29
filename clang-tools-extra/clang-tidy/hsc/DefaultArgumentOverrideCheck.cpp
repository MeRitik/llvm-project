#include "DefaultArgumentOverrideCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"

#include <cctype>
#include <string>

using namespace clang::ast_matchers;

namespace clang::tidy::hsc {

namespace {

std::string normalizeWhitespace(StringRef Text) {
  std::string Normalized;
  Normalized.reserve(Text.size());
  for (char C : Text)
    if (!std::isspace(static_cast<unsigned char>(C)))
      Normalized.push_back(C);
  return Normalized;
}

std::string getNormalizedDefaultArgText(const Expr *DefaultArg,
                                        const SourceManager &SM,
                                        const LangOptions &LangOpts) {
  if (!DefaultArg)
    return {};

  const CharSourceRange Range =
      CharSourceRange::getTokenRange(DefaultArg->getSourceRange());
  const StringRef Text = Lexer::getSourceText(Range, SM, LangOpts);
  if (Text.empty())
    return {};

  return normalizeWhitespace(Text);
}

bool hasDifferentDefaultArgument(const ParmVarDecl *OverrideParam,
                                 unsigned ParamIndex,
                                 const CXXMethodDecl *Override,
                                 const SourceManager &SM,
                                 const LangOptions &LangOpts) {
  const Expr *OverrideDefaultArg = OverrideParam->getDefaultArg();
  if (!OverrideDefaultArg)
    return false;

  const std::string OverrideText =
      getNormalizedDefaultArgText(OverrideDefaultArg, SM, LangOpts);

  for (const auto *BaseMethod : Override->overridden_methods()) {
    if (ParamIndex >= BaseMethod->param_size())
      continue;

    const ParmVarDecl *BaseParam = BaseMethod->getParamDecl(ParamIndex);
    if (!BaseParam->hasDefaultArg())
      return true;

    const std::string BaseText =
        getNormalizedDefaultArgText(BaseParam->getDefaultArg(), SM, LangOpts);
    if (BaseText != OverrideText)
      return true;
  }

  return false;
}

} // namespace

void DefaultArgumentOverrideCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxMethodDecl(unless(isImplicit()),
                    hasAnyParameter(parmVarDecl(hasDefaultArgument())))
          .bind("override"),
      this);
}

void DefaultArgumentOverrideCheck::check(
    const MatchFinder::MatchResult &Result) {
  const auto *Override = Result.Nodes.getNodeAs<CXXMethodDecl>("override");
  if (!Override || !Override->isFirstDecl() ||
      Override->size_overridden_methods() == 0 || !Result.SourceManager)
    return;

  for (unsigned I = 0; I < Override->param_size(); ++I) {
    const ParmVarDecl *OverrideParam = Override->getParamDecl(I);
    if (!OverrideParam->hasDefaultArg())
      continue;

    if (hasDifferentDefaultArgument(OverrideParam, I, Override,
                                    *Result.SourceManager, getLangOpts())) {
      diag(OverrideParam->getLocation(),
           "overriding virtual function parameter shall not specify a "
           "different default argument");
      break;
    }
  }
}

} // namespace clang::tidy::hsc
