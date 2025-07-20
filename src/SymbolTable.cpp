//
// Created by remsc on 13/06/2025.
//

#include "SymbolTable.h"
#include "AST.h"

void SymbolTable::registerGeneric(unique_ptr<AST> ast) const {
    // This line requires BlockAST to be fully defined for generics->statements.push_back
    generics->statements.push_back(move(ast));
}
