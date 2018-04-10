set(LLVM_LINK_COMPONENTS support) 
set(LLVM_USED_LIBS clangTooling clangBasic clangAST)


add_clang_executable(par dep.cpp)
target_link_libraries(par PRIVATE clangTooling clangBasic clangASTMatchers clangFrontend 
			clangSerialization clangDriver clangTooling clangParse clangSema
           clangAnalysis clangRewriteFrontend 
	   clangEdit clangAST clangLex clangBasic)
