set(LLVM_LINK_COMPONENTS support)

add_clang_executable(record-info
  record.cpp
  )
target_link_libraries(record-info
  PRIVATE
  clangTooling
  clangBasic
  clangAST
  clangASTMatchers
  clangLex
  clangFrontend
  )