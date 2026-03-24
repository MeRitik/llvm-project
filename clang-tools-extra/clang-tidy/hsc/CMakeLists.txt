set(LLVM_LINK_COMPONENTS
  FrontendOpenMP
  Support
  )

add_clang_library(clangTidyHSCModule STATIC
  HSCTidyModule.cpp
  UnusedReturnValueCheck.cpp

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
