#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_VOID_INTEGER_TO_POINTER_CAST_CHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_VOID_INTEGER_TO_POINTER_CAST_CHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::hsc {

class VoidIntegerToPointerCastCheck : public ClangTidyCheck {
public:
  VoidIntegerToPointerCastCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace clang::tidy::hsc

#endif
