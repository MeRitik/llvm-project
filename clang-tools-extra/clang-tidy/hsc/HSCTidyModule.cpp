//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "AdvancedMemoryManagementCheck.h"
#include "ConcealedBaseFunctionCheck.h"
#include "DefaultArgumentOverrideCheck.h"
#include "DependentBaseLookupCheck.h"
#include "DynamicMemoryManagementCheck.h"
#include "DynamicTypeInConstructorCheck.h"
#include "EscapingLocalAddressCheck.h"
#include "ExceptionPointerCheck.h"
#include "ExceptionUnfriendlyFunctionCheck.h"
#include "ExplicitBaseInitCheck.h"
#include "ExplicitSingleArgumentConstructorCheck.h"
#include "IfElseIfTerminationCheck.h"
#include "MemberInitializationCheck.h"
#include "MixedVirtualInheritanceCheck.h"
#include "MovedFromStateCheck.h"
#include "OverlappingCopyCheck.h"
#include "RefQualifiedThisReturnCheck.h"
#include "ReinterpretCastCheck.h"
#include "RemoveConstCheck.h"
#include "SelfAssignmentCheck.h"
#include "UnusedFunctionCheck.h"
#include "UnusedParameterCheck.h"
#include "UnusedReturnValueCheck.h"
#include "UnusedTypeCheck.h"
#include "UnusedVariableCheck.h"
#include "VirtualMemberPointerCmpCheck.h"
#include "VirtualOverrideCheck.h"

namespace clang::tidy {
namespace hsc {
namespace {

class HSCModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<AdvancedMemoryManagementCheck>(
        "hsc-advanced-memory-management");
    CheckFactories.registerCheck<ConcealedBaseFunctionCheck>(
        "hsc-concealed-base-function");
    CheckFactories.registerCheck<DefaultArgumentOverrideCheck>(
        "hsc-default-argument-override");
    CheckFactories.registerCheck<DependentBaseLookupCheck>(
        "hsc-dependent-base-lookup");
    CheckFactories.registerCheck<DynamicMemoryManagementCheck>(
        "hsc-dynamic-memory-management");
    CheckFactories.registerCheck<DynamicTypeInConstructorCheck>(
        "hsc-dynamic-type-in-constructor");
    CheckFactories.registerCheck<EscapingLocalAddressCheck>(
        "hsc-escaping-local-address");
    CheckFactories.registerCheck<ExceptionPointerCheck>(
        "hsc-exception-pointer");
    CheckFactories.registerCheck<ExceptionUnfriendlyFunctionCheck>(
        "hsc-exception-unfriendly-function");
    CheckFactories.registerCheck<ExplicitBaseInitCheck>(
        "hsc-explicit-base-init");
    CheckFactories.registerCheck<ExplicitSingleArgumentConstructorCheck>(
        "hsc-explicit-single-argument-constructor");
    CheckFactories.registerCheck<IfElseIfTerminationCheck>(
        "hsc-if-else-if-termination");
    CheckFactories.registerCheck<MemberInitializationCheck>(
        "hsc-member-initialization");
    CheckFactories.registerCheck<MixedVirtualInheritanceCheck>(
        "hsc-mixed-virtual-inheritance");
    CheckFactories.registerCheck<MovedFromStateCheck>("hsc-moved-from-state");
    CheckFactories.registerCheck<OverlappingCopyCheck>("hsc-overlapping-copy");
    CheckFactories.registerCheck<RefQualifiedThisReturnCheck>(
        "hsc-ref-qualified-this-return");
    CheckFactories.registerCheck<RemoveConstCheck>("hsc-remove-const");
    CheckFactories.registerCheck<ReinterpretCastCheck>("hsc-reinterpret-cast");
    CheckFactories.registerCheck<SelfAssignmentCheck>("hsc-self-assignment");
    CheckFactories.registerCheck<UnusedFunctionCheck>("hsc-unused-function");
    CheckFactories.registerCheck<UnusedParameterCheck>("hsc-unused-parameter");
    CheckFactories.registerCheck<UnusedReturnValueCheck>(
        "hsc-unused-return-value");
    CheckFactories.registerCheck<UnusedTypeCheck>("hsc-unused-type");
    CheckFactories.registerCheck<UnusedVariableCheck>("hsc-unused-variable");
    CheckFactories.registerCheck<VirtualMemberPointerCmpCheck>(
        "hsc-virtual-member-pointer-cmp");
    CheckFactories.registerCheck<VirtualOverrideCheck>("hsc-virtual-override");
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
