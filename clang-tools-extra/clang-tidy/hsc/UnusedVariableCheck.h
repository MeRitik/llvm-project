//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_UNUSEDVARIABLECHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_UNUSEDVARIABLECHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::hsc {

/// Detects limited-visibility variables that are never used.
/// Variables whose types have user-provided constructors or destructors are
/// considered used by side effects and are ignored.
class UnusedVariableCheck : public ClangTidyCheck {
public:
  UnusedVariableCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

  std::optional<TraversalKind> getCheckTraversalKind() const override {
    return TK_IgnoreUnlessSpelledInSource;
  }
};

} // namespace clang::tidy::hsc

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_HSC_UNUSEDVARIABLECHECK_H
