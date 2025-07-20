//
// Created by remsc on 20/07/2025.
//

#ifndef MONOMORPHIZER_H
#define MONOMORPHIZER_H

#include <string>
#include <map>
#include <memory>
#include "AST.h"

using namespace std;

/**
 * @brief Recursively visit the AST and replace generic types with concrete types
 *
 * Uses a substitution map<string, Type> : ex. "T" -> "int"
 *
 * @param node The parent node
 * @param typeMap The correspondance table
 */
void substitute_recursive(AST* node, const map<string, unique_ptr<TypeAST>>& typeMap);

/**
 * @brief Check if a type is instantiated, if not, create an instance according to the template
 * @param type The type to check
 * @param table
 * @return The type signature after instantiation
 */
string ensureTypeIsInstantiated(TypeAST* type, SymbolTable& table);

#endif //MONOMORPHIZER_H
