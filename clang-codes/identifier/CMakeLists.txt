set(LLVM_LINK_COMPONENTS support)

add_clang_executable(idt-info
  identifiers.cpp
  )
target_link_libraries(idt-info
  PRIVATE
  clangTooling
  clangBasic
  clangAST
  clangASTMatchers
  clangLex
  clangFrontend
  )