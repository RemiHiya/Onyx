//
// Created by remsc on 06/06/2025.
//

#ifndef PARSER_H
#define PARSER_H
#include <memory>
#include <utility>
#include <vector>

#include "AST.h"
#include "Lexer.h"

using namespace std;


class Parser {
    std::vector<Token> tokens;
    Token currentToken = tokens[0];
    int index = 0;
    void nextToken();
    Token peek(int offset);
public:
    void error(const std::string &message);
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}
    unique_ptr<BlockAST> parse();
    unique_ptr<BlockAST> parseBlock();
    unique_ptr<AST> parseStatement();
    void eat(TokenType type);
    unique_ptr<ExprAST> parseExpr();
    unique_ptr<ExprAST> parsePostfixExpr();
    unique_ptr<ExprAST> parseOpRHS(int exprPrecedence, unique_ptr<ExprAST> LHS);
    unique_ptr<ExprAST> parsePrimary();
    unique_ptr<ExprAST> parseUnaryExpr();
    unique_ptr<FloatExprAST> parseFloatExpr();
    unique_ptr<IntExprAST> parseIntExpr();
    unique_ptr<StringExprAST> parseString();
    unique_ptr<ExprAST> parseParenExpr();
    unique_ptr<ExprAST> parseIdentifierExpr();
    unique_ptr<ExternStatementAST> parseExternStatement();
    unique_ptr<TypeAST> parseType();
    unique_ptr<FunctionParameterAST> parseFunctionParameter();
    unique_ptr<FunctionDefinitionAST> parseFunctionDefinition(bool isStatic);
    unique_ptr<FunctionCallAST> parseFunctionCall();
    unique_ptr<MethodCallAST> parseMethodCall(bool isStatic);
    unique_ptr<VariableDeclarationAST> parseVariableDeclaration();
    unique_ptr<VariableAssignmentAST> parseVariableAssignment();
    unique_ptr<StructDefinitionAST> parseStructDefinition();
    unique_ptr<ConstructorDefinitionAST> parseConstructorDefinition();
    unique_ptr<ExtendsStatementAST> parseExtendsStatement();
    unique_ptr<ReturnAST> parseReturn();
    unique_ptr<IfStatementAST> parseIfStatement();
    unique_ptr<FunctionDefinitionAST> parseStaticDefinition();
};


#endif //PARSER_H
