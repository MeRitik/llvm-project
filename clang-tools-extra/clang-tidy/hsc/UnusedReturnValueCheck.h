//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_UNUSEDRETURNVALUECHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_UNUSEDRETURNVALUECHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::hsc {

/// Detects non-void function calls whose return value is ignored.
class UnusedReturnValueCheck : public ClangTidyCheck {
public:
  UnusedReturnValueCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

  std::optional<TraversalKind> getCheckTraversalKind() const override {
    return TK_IgnoreUnlessSpelledInSource;
  }
};

} // namespace clang::tidy::hsc

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_UNUSEDRETURNVALUECHECK_H
