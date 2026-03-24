//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "UnusedReturnValueCheck.h"
#include "UnusedVariableCheck.h"

namespace clang::tidy {
namespace hsc {
namespace {

class HSCModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<UnusedVariableCheck>("hsc-unused-variable");
    CheckFactories.registerCheck<UnusedReturnValueCheck>(
        "hsc-unused-return-value");
  }
};

} // namespace

// Register the HSCModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<HSCModule> X("hsc-module",
                                                 "Adds HSC-specific checks.");

} // namespace hsc

// This anchor is used to force the linker to link in the generated object file
// and thus register the HSCModule.
volatile int HSCModuleAnchorSource = 0; // NOLINT(misc-use-internal-linkage)

} // namespace clang::tidy
