set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_clang_library(clangTidyHSCModule STATIC
  ConcealedBaseFunctionCheck.cpp
  DependentBaseLookupCheck.cpp
  EscapingLocalAddressCheck.cpp
  ExplicitBaseInitCheck.cpp
  HSCTidyModule.cpp
  RefQualifiedThisReturnCheck.cpp
  UnusedReturnValueCheck.cpp
  UnusedVariableCheck.cpp
  UnusedTypeCheck.cpp
  UnusedFunctionCheck.cpp

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
