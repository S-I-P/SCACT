set(LLVM_LINK_COMPONENTS support)

add_clang_executable(clang-ast
  ClangAst.cpp
  )
target_link_libraries(clang-ast
  PRIVATE
  clangTooling
  clangBasic
  clangAST
  clangASTMatchers
  clangLex
  clangFrontend
  )