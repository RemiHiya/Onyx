//
// Created by remsc on 20/07/2025.
//

#include "AST.h"

unique_ptr<AST> BlockAST::clone() const {
    const auto clonedStatements = make_unique<vector<unique_ptr<AST>>>();
    for (const auto& stmt : statements) {
        clonedStatements->push_back(stmt->clone());
    }
    return make_unique<BlockAST>(move(*clonedStatements));
}

unique_ptr<AST> FloatExprAST::clone() const {
    return make_unique<FloatExprAST>(val);
}

unique_ptr<AST> IntExprAST::clone() const {
    return make_unique<IntExprAST>(val);
}

unique_ptr<AST> StringExprAST::clone() const {
    return make_unique<StringExprAST>(val);
}

unique_ptr<AST> VariableExprAST::clone() const {
    return make_unique<VariableExprAST>(name);
}

unique_ptr<AST> OperationExprAST::clone() const {
    auto clonedLHS = unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(LHS->clone().release()));
    auto clonedRHS = unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(RHS->clone().release()));
    return make_unique<OperationExprAST>(op, move(clonedLHS), move(clonedRHS));
}

unique_ptr<AST> ExternStatementAST::clone() const {
    return make_unique<ExternStatementAST>(libraryName);
}

unique_ptr<AST> TypeAST::clone() const {
    auto clonedArgs = vector<unique_ptr<TypeAST>>();
    for (const auto& arg : genericArgs) {
        clonedArgs.push_back(unique_ptr<TypeAST>(dynamic_cast<TypeAST*>(arg->clone().release())));
    }
    auto clonedSize = arraySize ? unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(arraySize->clone().release())) : nullptr;
    auto clonedType = make_unique<TypeAST>(type, move(clonedArgs));
    clonedType->isArray = isArray;
    clonedType->arraySize = move(clonedSize);
    return clonedType;
}

unique_ptr<AST> FunctionParameterAST::clone() const {
    auto clonedType = unique_ptr<TypeAST>(dynamic_cast<TypeAST*>(type->clone().release()));
    return make_unique<FunctionParameterAST>(move(clonedType), name);
}

unique_ptr<AST> FunctionDefinitionAST::clone() const {
    auto clonedReturnType = unique_ptr<TypeAST>(dynamic_cast<TypeAST*>(returnType->clone().release()));
    auto clonedParams = vector<unique_ptr<FunctionParameterAST>>();
    for (const auto& p : params) {
        clonedParams.push_back(unique_ptr<FunctionParameterAST>(dynamic_cast<FunctionParameterAST*>(p->clone().release())));
    }
    auto clonedBody = body->clone();
    return make_unique<FunctionDefinitionAST>(move(clonedReturnType), name, move(clonedParams), move(clonedBody), isStatic);
}

unique_ptr<AST> FunctionCallAST::clone() const {
    auto clonedParams = vector<unique_ptr<ExprAST>>();
    for (const auto& p : params) {
        clonedParams.push_back(unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(p->clone().release())));
    }
    return make_unique<FunctionCallAST>(name, move(clonedParams));
}

unique_ptr<AST> MethodCallAST::clone() const {
    auto clonedOwner = unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(ownerExpr->clone().release()));
    auto clonedParams = vector<unique_ptr<ExprAST>>();
    for (const auto& p : params) {
        clonedParams.push_back(unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(p->clone().release())));
    }
    return make_unique<MethodCallAST>(move(clonedOwner), name, move(clonedParams));
}

unique_ptr<AST> FieldAccessAST::clone() const {
    auto clonedOwner = unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(ownerExpr->clone().release()));
    return make_unique<FieldAccessAST>(move(clonedOwner), name);
}

unique_ptr<AST> VariableDeclarationAST::clone() const {
    auto clonedType = unique_ptr<TypeAST>(dynamic_cast<TypeAST*>(type->clone().release()));
    auto clonedInitializer = initializer ? unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(initializer->clone().release())) : nullptr;
    return make_unique<VariableDeclarationAST>(move(clonedType), name, move(clonedInitializer));
}

unique_ptr<AST> VariableAssignmentAST::clone() const {
    auto clonedTarget = unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(target->clone().release()));
    auto clonedValue = unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(value->clone().release()));
    return make_unique<VariableAssignmentAST>(move(clonedTarget), move(clonedValue));
}

unique_ptr<AST> GenericParameterAST::clone() const {
    return make_unique<GenericParameterAST>(name);
}

unique_ptr<AST> StructFieldAST::clone() const {
    auto clonedType = unique_ptr<TypeAST>(dynamic_cast<TypeAST*>(type->clone().release()));
    return make_unique<StructFieldAST>(move(clonedType), name);
}

unique_ptr<AST> StructDefinitionAST::clone() const {
    auto clonedGenericParams = vector<unique_ptr<GenericParameterAST>>();
    for (const auto& p : genericParams) {
        clonedGenericParams.push_back(unique_ptr<GenericParameterAST>(dynamic_cast<GenericParameterAST*>(p->clone().release())));
    }
    auto clonedFields = vector<unique_ptr<StructFieldAST>>();
    for (const auto& f : fields) {
        clonedFields.push_back(unique_ptr<StructFieldAST>(dynamic_cast<StructFieldAST*>(f->clone().release())));
    }
    return make_unique<StructDefinitionAST>(name, move(clonedGenericParams), move(clonedFields));
}

unique_ptr<AST> ConstructorDefinitionAST::clone() const {
    auto clonedParams = vector<unique_ptr<FunctionParameterAST>>();
    for (const auto& p : params) {
        clonedParams.push_back(unique_ptr<FunctionParameterAST>(dynamic_cast<FunctionParameterAST*>(p->clone().release())));
    }
    auto clonedBody = body->clone();
    return make_unique<ConstructorDefinitionAST>(structName, move(clonedParams), move(clonedBody));
}

unique_ptr<AST> ExtendsStatementAST::clone() const {
    auto clonedMembers = vector<unique_ptr<AST>>();
    for (const auto& member : members) {
        clonedMembers.push_back(member->clone());
    }
    return make_unique<ExtendsStatementAST>(structName, parentStructName, move(clonedMembers));
}

unique_ptr<AST> ReturnAST::clone() const {
    auto clonedValue = unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(value->clone().release()));
    return make_unique<ReturnAST>(move(clonedValue));
}

unique_ptr<AST> IfStatementAST::clone() const {
    auto clonedCondition = unique_ptr<ExprAST>(dynamic_cast<ExprAST*>(condition->clone().release()));
    auto clonedThen = thenBody->clone();
    auto clonedElse = elseBody ? elseBody->clone() : nullptr;
    return make_unique<IfStatementAST>(move(clonedCondition), move(clonedThen), move(clonedElse));
}

unique_ptr<AST> ExternExprAST::clone() const {
    return make_unique<ExternExprAST>(body);
}