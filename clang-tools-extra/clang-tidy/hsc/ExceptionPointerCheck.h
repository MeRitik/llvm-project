#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_EXCEPTIONPOINTER_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_EXCEPTIONPOINTER_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::hsc {
class ExceptionPointerCheck : public ClangTidyCheck {
public:
  ExceptionPointerCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};
} // namespace clang::tidy::hsc
#endif
