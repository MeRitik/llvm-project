#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_UNUSEDPARAMETERCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_UNUSEDPARAMETERCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::hsc {
class UnusedParameterCheck : public ClangTidyCheck {
public:
  UnusedParameterCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};
} // namespace clang::tidy::hsc
#endif
