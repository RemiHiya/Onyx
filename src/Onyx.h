//
// Created by remsc on 12/06/2025.
//

#ifndef ONYX_H
#define ONYX_H
#include <memory>
#include <string>

#include "AST.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define OS_WINDOWS
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(__ANDROID__)
    #define OS_UNIX_LIKE
#else
    #error "Unknown or unsupported operating system"
#endif

class Onyx {
    bool success = true;
public:
    vector<string> visited;
    optional<string> Compile(const string &sourcefile);
    unique_ptr<BlockAST> BuildAST(const string& sourcefile);
    map<string, unique_ptr<BlockAST>> BuildASTMap(const string& sourcefile);
    void AnalyseAST(const std::unique_ptr<BlockAST> &ast);
};

#endif //ONYX_H
