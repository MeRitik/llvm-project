//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "ConcealedBaseFunctionCheck.h"
#include "DependentBaseLookupCheck.h"
#include "EscapingLocalAddressCheck.h"
#include "ExplicitBaseInitCheck.h"
#include "RefQualifiedThisReturnCheck.h"
#include "UnusedFunctionCheck.h"
#include "UnusedReturnValueCheck.h"
#include "UnusedTypeCheck.h"
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
    CheckFactories.registerCheck<UnusedTypeCheck>("hsc-unused-type");
    CheckFactories.registerCheck<UnusedFunctionCheck>("hsc-unused-function");
    CheckFactories.registerCheck<ConcealedBaseFunctionCheck>(
        "hsc-concealed-base-function");
    CheckFactories.registerCheck<DependentBaseLookupCheck>(
        "hsc-dependent-base-lookup");
    CheckFactories.registerCheck<EscapingLocalAddressCheck>(
        "hsc-escaping-local-address");
    CheckFactories.registerCheck<RefQualifiedThisReturnCheck>(
        "hsc-ref-qualified-this-return");
    CheckFactories.registerCheck<ExplicitBaseInitCheck>(
        "hsc-explicit-base-init");
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
