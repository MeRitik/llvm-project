set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_clang_library(clangTidyHSCModule STATIC
  AdvancedMemoryManagementCheck.cpp
  CStyleFunctionalCastCheck.cpp
  ConcealedBaseFunctionCheck.cpp
  DefaultArgumentOverrideCheck.cpp
  DependentBaseLookupCheck.cpp
  DynamicMemoryManagementCheck.cpp
  DynamicTypeInConstructorCheck.cpp
  EscapingLocalAddressCheck.cpp
  ExceptionPointerCheck.cpp
  ExceptionUnfriendlyFunctionCheck.cpp
  ExplicitBaseInitCheck.cpp
  ExplicitSingleArgumentConstructorCheck.cpp
  IfElseIfTerminationCheck.cpp
  HSCTidyModule.cpp
  MemberInitializationCheck.cpp
  MixedVirtualInheritanceCheck.cpp
  MovedFromStateCheck.cpp
  OverlappingCopyCheck.cpp
  RefQualifiedThisReturnCheck.cpp
  RemoveConstCheck.cpp
  ReinterpretCastCheck.cpp
  SelfAssignmentCheck.cpp
  UnusedFunctionCheck.cpp
  UnusedParameterCheck.cpp
  UnusedReturnValueCheck.cpp
  UnusedTypeCheck.cpp
  UnusedVariableCheck.cpp
  VoidIntegerToPointerCastCheck.cpp
  VirtualMemberPointerCmpCheck.cpp
  VirtualBaseToDerivedCastCheck.cpp
  VirtualOverrideCheck.cpp

  LINK_LIBS
  clangTidy
  clangTidyUtils

  DEPENDS
  omp_gen
  ClangDriverOptions
  )

clang_target_link_libraries(clangTidyHSCModule
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  )
