cmake_minimum_required(VERSION 3.31)
project(Onyx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(LLVM 20.1.7 REQUIRED CONFIG)

add_executable(Onyx src/main.cpp
        src/Lexer.cpp
        src/Lexer.h
        src/Parser.cpp
        src/Parser.h
        src/AST.cpp
        src/AST.h
        src/Logger.cpp
        src/Logger.h
        src/Onyx.cpp
        src/Onyx.h
        src/SymbolTable.cpp
        src/SymbolTable.h
        src/CodeGenerator.cpp
        src/CodeGenerator.h
        src/IR/memory.c
        src/IR/memory.h
        build/builtins.h
        src/CloneAST.cpp
        # should be removed later
        # ---
)

#[[
target_link_libraries(Onyx PRIVATE
        LLVM::Core
        LLVM::Support
        LLVM::Analysis
        LLVM::CodeGen
        LLVM::Target
        LLVM::TargetParser
)]]
#target_link_libraries(Onyx PRIVATE ${LLVM_LIBRARIES})
#include(LLVMInitializeTargets)
#target_link_libraries(Onyx PRIVATE
#        LLVM::AllTargets
#)
#target_include_directories(Onyx PRIVATE ${LLVM_INCLUDE_DIRS})
#target_compile_definitions(Onyx PRIVATE ${LLVM_DEFINITIONS})