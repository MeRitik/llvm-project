#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_VIRTUAL_BASE_TO_DERIVED_CAST_CHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_VIRTUAL_BASE_TO_DERIVED_CAST_CHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::hsc {

class VirtualBaseToDerivedCastCheck : public ClangTidyCheck {
public:
  VirtualBaseToDerivedCastCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace clang::tidy::hsc

#endif