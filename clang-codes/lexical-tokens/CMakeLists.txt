set(LLVM_LINK_COMPONENTS support)

add_clang_executable(lex-tok
  LexicalTokens.cpp
  )
target_link_libraries(lex-tok
  PRIVATE
  clangTooling
  clangBasic
  clangAST
  clangASTMatchers
  clangLex
  clangFrontend
  )