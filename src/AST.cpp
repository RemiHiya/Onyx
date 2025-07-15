//
// Created by remsc on 07/06/2025.
//

#include "AST.h"

#include <format>
#include <set>

#include "Logger.h"

// Hoisting
void BlockAST::prePass(SymbolTable &table) {
    for (auto& stmt : statements) {
        if (auto* funcDef = dynamic_cast<FunctionDefinitionAST*>(stmt.get())) {
            funcDef->prePass(table);
        }
        if (auto* structDef = dynamic_cast<StructDefinitionAST*>(stmt.get())) {
           structDef->prePass(table);
        }
    }
}

// Entry point of the semantic analysis
void BlockAST::analyse(SymbolTable &table) {
    table.enterScope();
    for (const auto& stmt : statements) {
        stmt->analyse(table);
    }
    table.exitScope();
}

string BlockAST::code() {
    string code = "{\n";
    for (const auto& stmt : statements) {
        code += stmt->code() + ";\n";
    }
    code += "}\n";
    return code;
}

void ExprAST::prePass(SymbolTable &table) {
    AST::prePass(table);
}

void FloatExprAST::analyse(SymbolTable &table, string &a) {
    a = "float";
}

string FloatExprAST::code() {
    return to_string(val) + "f";
}

void IntExprAST::analyse(SymbolTable &table, string &a) {
    a = "int";
}

string IntExprAST::code() {
    return to_string(val);
}

void StringExprAST::analyse(SymbolTable &table, string &a) {
    a = "string";
}

string StringExprAST::code() {
    return '"' + val + '"';
}

void VariableExprAST::analyse(SymbolTable &table, string &a) {
    analyse(table, a, "");
}

void VariableExprAST::analyse(SymbolTable &table, string &a, const std::string &currentStruct) {
    if (const auto symbol = table.lookupSymbol(name)) {
        a = symbol->type;
        isField = false;
        return;
    }
    if (!currentStruct.empty()) {
        if (const auto fieldSymbol = table.lookupField(currentStruct, name)) {
            a = fieldSymbol->type;
            isField = true;
            return;
        }
    }
    Logger::Error("Variable '" + name + "' not declared.");
    a = "error_type";
}

string VariableExprAST::code() {
    if (isField) {
        return "self->" + name;
    }
    return name;
}

void OperationExprAST::analyse(SymbolTable &table, string &a) {
    string lhsType;
    string rhsType;
    LHS->analyse(table, lhsType);
    RHS->analyse(table, rhsType);

    if (lhsType != rhsType) {
        Logger::Error("Type mismatch in operation. LHS is '" + lhsType + "', RHS is '" + rhsType + "'.");
        a = "error_type";
    }
    a = lhsType;
}

string OperationExprAST::code() {
    string ope = tokenToString(op);
    return LHS->code() + ' ' + ope + ' ' + RHS->code();
}

// REWORK : maybe add other primitives
bool isPrimitive(const string& type) {
    const set<string> primitives = {"int", "float", "double", "bool"};
    return primitives.contains(type);
}

string TypeAST::code() {
    if (isPrimitive(type)) {
        return type;
    }
    return type + "*";
}


string FunctionParameterAST::code() {
    return type->code() + ' ' + name;
}

void FunctionDefinitionAST::prePass(SymbolTable &table) {
    if (const string signature = getSignature(); !table.addSymbol(signature, {returnType->type})) {
        Logger::Error("Function '" + name + "' already defined. (signature : "+signature+").");
    }
}

void FunctionDefinitionAST::analyse(SymbolTable &table) {
    analyse(table, "");
}

void FunctionDefinitionAST::analyse(SymbolTable& table, string parentStruct) {
    if (!table.addSymbol(getSignature(), {returnType->type})) {
        Logger::Error("Function '" + name + "' already defined. (signature : " + getSignature() + ")");
    }
    table.enterScope();

    for (const auto& param : params) {
        if (!table.addSymbol(param->name, {param->type->type})) {
            Logger::Error("Parameter '" + param->name + "' already defined in function '" + name + "'.");
        }
    }

    // Lookup for the return type of the function
    string type;
    if (const auto assignment = dynamic_cast<ExprAST*>(body.get())) {
        assignment->analyse(table, type);
        if (type != returnType->type) {
            Logger::Error("Invalid return type, expected '"+returnType->type+"' but found '"+type+"'.");
        }
    }
    else if (const auto block = dynamic_cast<BlockAST*>(body.get())) {
        // TODO : void functions
        for (const auto& stmt : block->statements) {
            if (const auto ret = dynamic_cast<ReturnAST*>(stmt.get())) {
                auto* value = ret->value.get();
                auto* var = dynamic_cast<VariableExprAST*>(value);

                if (const auto* field = dynamic_cast<FieldAccessAST*>(value); var && !field) {
                    var->analyse(table, type, parentStruct);
                } else {
                    value->analyse(table, type);
                }

                if (type != returnType->type) {
                    Logger::Error("Invalid return type, expected '" + returnType->type +
                                  "' but found '" + type + "'.");
                }

                continue;
            }
            if (const auto var = dynamic_cast<VariableExprAST*>(stmt.get())) {
                string tmp;
                var->analyse(table, tmp, parentStruct);
            } else if (const auto expr = dynamic_cast<ExprAST*>(stmt.get())){
                string tmp;
                expr->analyse(table, tmp);
            } else {
                stmt->analyse(table);
            }
        }
    }
    if (type.empty()) {
        Logger::Error("Missing return statement in function '" + name +"'.");
    }

    // TODO : check for return
    table.exitScope();
}

string FunctionDefinitionAST::code(bool isMethod) {
    string code = returnType->code() + ' ' + getSignature() + '(';
    if (isMethod) {
        code += "* self";
        if (!params.empty()) {
            code += ", ";
        }
    }
    for (const auto& param : params) {
        code += param->code();
        if (param != params.back()) {
            code += ", ";
        }
    }
    code += ')';
    if (const auto expr = dynamic_cast<ExprAST*>(body.get())) {
        code += "{ return "+ expr->code() +"; }";
    } else if (const auto block = dynamic_cast<BlockAST*>(body.get())) {
        if (name == "main") {
            code += "{\n";
            code += "\tinitGlobalPool(0, 0);";
            for (const auto& stmt : block->statements) {
                code += '\t' + stmt->code() + ";\n";
            }
            code += "}\n";
        } else {
            code += block->code();
        }
    }

    return code;
}

string FunctionDefinitionAST::code() {
    return code(false);
}

string FunctionDefinitionAST::getSignature() {
    string signature = (name !="main" ? "fun_" : "") + name;
    for (const auto& p : params) {
        signature += '_' + p->type->type;
    }
    return signature;
}

string StructFieldAST::code() {
    return type->code() + ' ' + name;
}

void StructDefinitionAST::prePass(SymbolTable &table) {
    if (!table.addSymbol(name, {name})) {
        Logger::Error("Structure '" + name + "' already defined.");
    }
}

void StructDefinitionAST::analyse(SymbolTable &table) {
    table.enterScope();
    string sign;

    for (const auto& generic : genericParams) {
        if (!table.addSymbol(generic->name, {"generic", SymbolInfo::Generic})) {
            Logger::Error("Generic parameter '" + generic->name + "' already defined in the current structure '" + name + "'.");
        }
    }
    for (const auto& field : fields) {
        if (!table.addSymbol(field->name, {field->type->type, SymbolInfo::Variable})) {
            Logger::Error("Field '" + field->name + "' already defined in the current structure '" + name + "'.");
            continue;
        }
        sign += "_" + field->type->type;
    }
    StructFieldMap fieldsMap;
    for (const auto& field : fields) {
        fieldsMap[field->name] = {field->type->type, SymbolInfo::Variable};
    }
    table.addStruct(name, fieldsMap);

    table.exitScope();
    table.addSymbol("fun_" + name + sign, {name});
}

void ConstructorDefinitionAST::prePass(SymbolTable &table) {
    // Le constructeur est une fonction qui retourne un pointeur vers la struct
    if (!table.addSymbol(getSignature(), {structName + "*", SymbolInfo::Function})) {
        Logger::Error("Constructor for struct '" + structName + "' with this signature already defined.");
    }
}

void ConstructorDefinitionAST::analyse(SymbolTable &table) {
    table.enterScope();
    for (const auto& param : params) {
        table.addSymbol(param->name, {param->type->type, SymbolInfo::Variable});
    }
    body->analyse(table);
    table.exitScope();
}

string ConstructorDefinitionAST::getSignature() {
    // Signature : StructName_new_paramType1_paramType2
    string signature = structName + "_new";
    for (const auto& p : params) {
        signature += '_' + p->type->type;
    }
    return signature;
}

string ConstructorDefinitionAST::code() {
    string signature = getSignature();
    string code = structName + "* " + signature + "(";
    for (const auto& param : params) {
        code += param->code();
        if (param != params.back()) {
            code += ", ";
        }
    }
    code += ") ";

    string bodyCode = "{\n";
    // Alloc of the struct
    bodyCode += "\t" + structName + "* self = alloc(sizeof(" + structName + "));\n";

    if (const auto block = dynamic_cast<BlockAST*>(body.get())) {
        for (const auto& stmt : block->statements) {
            bodyCode += "\t" + stmt->code();
        }
    }

    bodyCode += "\treturn self;\n";
    bodyCode += "}\n";

    return code + bodyCode;
}

void ExtendsStatementAST::analyse(SymbolTable& table) {
    for (auto& member : members) {
        if (auto* method = dynamic_cast<FunctionDefinitionAST*>(member.get())) {
            method->analyse(table, structName);
            if (!table.addSymbol(structName + '_' + method->getSignature(), {method->returnType->type})) {
                Logger::Error("Method " + method->name + " already defined in struct " + structName + ".");
            }
        }
        // TODO : else analyse case
    }
}

bool ExtendsStatementAST::isFieldOnly() {
    for (auto& stmt : members) {
        if (dynamic_cast<VariableDeclarationAST*>(stmt.get()) == nullptr) {
            return false;
        }
    }
    return true;
}

string ReturnAST::code() {
    return "return " + value->code();
}

string ExternExprAST::code() {
    return body;
}

void FunctionCallAST::analyse(SymbolTable &table, string &a) {
    // Check if it's a constructor call
    optional<SymbolInfo> typeInfo = table.lookupSymbol(name);
    if (typeInfo && typeInfo->metaType == SymbolInfo::Structure) {
        signature = name + "_new";
        for (const auto& param : params) {
            string tmp;
            param->analyse(table, tmp);
            signature += '_' + tmp;
        }

        const auto symbol = table.lookupSymbol(signature);
        if (!symbol || symbol->metaType != SymbolInfo::Function) {
            Logger::Error("Constructor for '" + name + "' not declared with signature: " + signature);
            a = "error_type";
            return;
        }
        a = name + "*";
        return;
    }

    // Normal function call
    string sign = "fun_" + name;
    for (const auto& param : params) {
        string tmp;
        param->analyse(table, tmp);
        sign += '_' + tmp;
    }
    signature = sign;

    const auto symbol = table.lookupSymbol(signature);
    if (!symbol || symbol->metaType != SymbolInfo::Function) {
        Logger::Error("Function '" + name + "' not declared. (signature : "+signature+").");
        a = "error_type";
    } else {
        a = symbol->type;
    }
}

string FunctionCallAST::code() {
    string code = signature + '(';
    for (auto& param : params) {
        code += param->code();
        if (param != params.back()) {
            code += ", ";
        }
    }
    return code + ')';
}

// REWORK : check object
void MethodCallAST::analyse(SymbolTable &table, string &a) {
    // Analyse the owner type
    string ownerType;
    ownerExpr->analyse(table, ownerType);

    if (ownerType == "error_type") {
        a = "error_type";
        return;
    }

    // Extract the type (without the pointer symbol)
    if (!ownerType.empty() && ownerType.back() == '*') {
        ownerType.pop_back();
    }
    signature = ownerType + "_fun_" + name;
    for (const auto& param : params) {
        string tmp;
        param->analyse(table, tmp);
        signature += '_' + tmp;
    }

    // Check if the method acually exists
    const auto symbol = table.lookupSymbol(signature);
    if (!symbol || symbol->metaType != SymbolInfo::Function) {
        Logger::Error("Method '" + name + "' not declared. (signature : "+signature+").");
        a = "error_type";
        return;
    }
    a = symbol->type;
}

string MethodCallAST::code() {
    string code = signature + '(' + ownerExpr->code();
    if (!params.empty()) {
        code += ", ";
    }
    for (auto& param : params) {
        code += param->code();
        if (param != params.back()) {
            code += ", ";
        }
    }
    return code + ')';
}

void FieldAccessAST::analyse(SymbolTable &table, string &a) {
    string ownerType;
    ownerExpr->analyse(table, ownerType);
    if (ownerType == "error_type") {
        a = "error_type";
        return;
    }

    // Extract the type (without the pointer symbol)
    if (!ownerType.empty() && ownerType.back() == '*') {
        ownerType.pop_back();
    }

    if (const auto fieldSymbol = table.lookupField(ownerType, name)) {
        a = fieldSymbol->type;
    } else {
        Logger::Error("Field '" + name + "' does not exist in struct '" + ownerType + "'.");
        a = "error_type";
    }
}



string FieldAccessAST::code() {
    string code = ownerExpr->code();
    return ownerExpr->code() + "->" + name;
}

void VariableDeclarationAST::analyse(SymbolTable &table) {
    string initType;
    if (initializer) {
        initializer->analyse(table, initType);
    }

    // TODO : check if the type exists
    //if (table.lookupSymbol(type->type) == nullopt) {
    //    Logger::Error("Type '" + type->type + "' does not exists.");
    //    return;
    //}
    const optional<SymbolInfo> lookup = table.lookupSymbol(name);
    if (lookup.has_value() && lookup.value().metaType == SymbolInfo::Variable) {
        Logger::Error("Variable '" + name + "' already defined in this scope.");
        return;
    }
    if (initializer && type->type != initType) {
        Logger::Error("Type mismatch in variable declaration '" + name + "'. Expected '" + type->type + "' but got '" + initType + "'.");
        return;
    }

    table.addSymbol(name, {type->type, SymbolInfo::Variable});
}

string VariableDeclarationAST::code() {
    if (initializer)
        return type->code() + ' ' + name + '=' + initializer->code();
    return type->code() + ' ' + name;
}

void VariableAssignmentAST::analyse(SymbolTable &table) {
    const optional<SymbolInfo> lookup = table.lookupSymbol(name);
    if (!lookup.has_value()) {
        Logger::Error("Variable '"+name+"' does not exists.");
    }
    string assignment;
    value->analyse(table, assignment);
    if (lookup.value().metaType != SymbolInfo::Variable || assignment != lookup.value().type) {
        Logger::Error("Type mismatch in variable assignment '" + name + "'. Expected '" + lookup.value().type + "' but got '" + assignment + "'.");
    }
}


