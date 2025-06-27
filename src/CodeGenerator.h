//
// Created by remsc on 18/06/2025.
//

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include <memory>

#include "AST.h"


class CodeGenerator {
    unique_ptr<AST> ast;
    string implementation;
public:
    explicit CodeGenerator(unique_ptr<AST> ast) : ast(move(ast)) {}

    string generate() const;
    string generateHeader();
};



#endif //CODEGENERATOR_H
