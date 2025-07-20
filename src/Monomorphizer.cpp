//
// Created by remsc on 20/07/2025.
//

#include "Monomorphizer.h"
#include <vector>

#include "Logger.h"

/**
 * @brief Check if a type is instantiated, if not, create an instance according to the template
 * @param type The type to check
 * @param table
 * @return The type signature after instantiation
 */
string ensureTypeIsInstantiated(TypeAST* type, SymbolTable& table) {
    if (type->genericArgs.empty()) {
        return type->type; // Simple type
    }

    const string mangledName = type->getMangledName();
    if (auto concreteName = table.findInstantiation(mangledName)) {
        return *concreteName; // Type already instantiated
    }

    // --- INSTANTIATION ---
    const auto* templateAST = table.lookupTemplate(type->type); // Get the AST associated to the template
    if (!templateAST) {
        Logger::Error("Generic type '" + type->type + "' not found.");
        return "error_type";
    }

    // 1. AST cloning
    auto clonedASTNode = templateAST->clone();
    auto* clonedStruct = dynamic_cast<StructDefinitionAST*>(clonedASTNode.get());

    // 2. Creating map that associate generic parameter with a concrete type
    map<string, unique_ptr<TypeAST>> typeMap;
    for (size_t i = 0; i < templateAST->genericParams.size(); ++i) {
        auto clonedArg = type->genericArgs[i]->clone();
        typeMap[templateAST->genericParams[i]->name] = unique_ptr<TypeAST>(dynamic_cast<TypeAST*>(clonedArg.release()));
    }

    // 3. Substitute types in the AST
    substitute_recursive(clonedStruct, typeMap);

    // 4. Generate an identifier for the type
    string concreteName = type->type;
    for(const auto& arg : type->genericArgs) {
        concreteName += "_" + arg->type;
    }
    clonedStruct->name = concreteName;

    // 5. Analyse the generated type
    clonedStruct->prePass(table);
    clonedStruct->analyse(table);

    // 6. Register the generated type
    table.registerGeneric(move(clonedASTNode));
    //globalBlock.statements.push_back(move(clonedASTNode));
    table.addInstantiation(mangledName);

    return concreteName;
}


void substitute_type(unique_ptr<TypeAST>& type, const map<string, unique_ptr<TypeAST>>& typeMap);

void substitute_recursive(AST* node, const map<string, unique_ptr<TypeAST>>& typeMap) {
    if (!node) {
        return;
    }

    // --- Structures & Functions ---

    if (auto* structDef = dynamic_cast<StructDefinitionAST*>(node)) {
        // Clear all generics parameters
        structDef->genericParams.clear();
        for (auto& field : structDef->fields) {
            substitute_recursive(field.get(), typeMap);
        }
        return;
    }
    if (auto* field = dynamic_cast<StructFieldAST*>(node)) {
        substitute_type(field->type, typeMap);
        return;
    }
    if (auto* funcDef = dynamic_cast<FunctionDefinitionAST*>(node)) {
        substitute_type(funcDef->returnType, typeMap);
        for (auto& param : funcDef->params) {
            substitute_recursive(param.get(), typeMap);
        }
        substitute_recursive(funcDef->body.get(), typeMap);
        return;
    }
    if (const auto* ctorDef = dynamic_cast<ConstructorDefinitionAST*>(node)) {
        for (auto& param : ctorDef->params) {
            substitute_recursive(param.get(), typeMap);
        }
        substitute_recursive(ctorDef->body.get(), typeMap);
        return;
    }

    if (auto* param = dynamic_cast<FunctionParameterAST*>(node)) {
        substitute_type(param->type, typeMap);
        return;
    }

    if (const auto* extendsStmt = dynamic_cast<ExtendsStatementAST*>(node)) {
        for (auto& member : extendsStmt->members) {
            substitute_recursive(member.get(), typeMap);
        }
        return;
    }

    // --- Statements ---

    if (const auto* block = dynamic_cast<BlockAST*>(node)) {
        for (auto& stmt : block->statements) {
            substitute_recursive(stmt.get(), typeMap);
        }
        return;
    }

    if (auto* varDecl = dynamic_cast<VariableDeclarationAST*>(node)) {
        substitute_type(varDecl->type, typeMap);
        if (varDecl->initializer) {
            substitute_recursive(varDecl->initializer.get(), typeMap);
        }
        return;
    }

    if (const auto* varAssign = dynamic_cast<VariableAssignmentAST*>(node)) {
        substitute_recursive(varAssign->target.get(), typeMap);
        substitute_recursive(varAssign->value.get(), typeMap);
        return;
    }

    if (const auto* ret = dynamic_cast<ReturnAST*>(node)) {
        substitute_recursive(ret->value.get(), typeMap);
        return;
    }

    if (const auto* ifStmt = dynamic_cast<IfStatementAST*>(node)) {
        substitute_recursive(ifStmt->condition.get(), typeMap);
        substitute_recursive(ifStmt->thenBody.get(), typeMap);
        if (ifStmt->elseBody) {
            substitute_recursive(ifStmt->elseBody.get(), typeMap);
        }
        return;
    }

    // --- Expressions ---

    if (const auto* op = dynamic_cast<OperationExprAST*>(node)) {
        substitute_recursive(op->LHS.get(), typeMap);
        substitute_recursive(op->RHS.get(), typeMap);
        return;
    }

    if (const auto* call = dynamic_cast<FunctionCallAST*>(node)) {
        for (auto& arg : call->params) {
            substitute_recursive(arg.get(), typeMap);
        }
        return;
    }

    if (const auto* methodCall = dynamic_cast<MethodCallAST*>(node)) {
        substitute_recursive(methodCall->ownerExpr.get(), typeMap);
        for (auto& arg : methodCall->params) {
            substitute_recursive(arg.get(), typeMap);
        }
        return;
    }

    if (const auto* fieldAccess = dynamic_cast<FieldAccessAST*>(node)) {
        substitute_recursive(fieldAccess->ownerExpr.get(), typeMap);
    }
}

void substitute_type(unique_ptr<TypeAST>& type, const map<string, unique_ptr<TypeAST>>& typeMap) {
    if (!type) return;

    // Current type is a generic
    if (typeMap.contains(type->type)) {
        type = unique_ptr<TypeAST>(dynamic_cast<TypeAST*>(typeMap.at(type->type)->clone().release()));
        return;
    }

    // Current type uses generics
    for (auto& arg : type->genericArgs) {
        substitute_type(arg, typeMap);
    }
}
