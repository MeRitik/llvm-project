#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_DEFAULTARGUMENTOVERRIDE_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_DEFAULTARGUMENTOVERRIDE_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::hsc {
class DefaultArgumentOverrideCheck : public ClangTidyCheck {
public:
  DefaultArgumentOverrideCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};
} // namespace clang::tidy::hsc
#endif
