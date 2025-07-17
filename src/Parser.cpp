//
// Created by remsc on 06/06/2025.
//

#include "Parser.h"

#include <iostream>
#include <map>

#include "Logger.h"

static int getTokenPrecedence(TokenType t) {
    const std::map<TokenType, int> precedence = {
        // Unary should be tested in parse primary, useless definition here
        //{TokenType::T_Increment,    100}, // ++
        //{TokenType::T_Decrement,    100}, // --
        //{TokenType::T_Not,          100}, // !
        //{TokenType::T_Complement,   100}, // ~

        {TokenType::T_Mul,          90}, // *
        {TokenType::T_Div,          90}, // /
        {TokenType::T_Mod,          90}, // %

        {TokenType::T_Add,          80}, // +
        {TokenType::T_Sub,          80}, // -

        {TokenType::T_LBitShift,    70}, // <<
        {TokenType::T_RBitShift,    70}, // >>

        {TokenType::T_GT,           60}, // >
        {TokenType::T_GE,           60}, // >=
        {TokenType::T_LT,           60}, // <
        {TokenType::T_LE,           60}, // <=

        {TokenType::T_Equals,       50}, // ==
        {TokenType::T_NotEquals,    50}, // !=

        {TokenType::T_BitAND,       40}, // &
        {TokenType::T_BitXOR,       30}, // ^
        {TokenType::T_BitOR,        20}, // |

        {TokenType::T_LogAND,       15}, // &&
        {TokenType::T_LogOR,        10}, // ||

        {TokenType::T_Assign,       5},
        {TokenType::T_AddAssign,    5},
        {TokenType::T_SubAssign,    5},
        {TokenType::T_MulAssign,    5},
        {TokenType::T_DivAssign,    5},
        {TokenType::T_ModAssign,    5},
        {TokenType::T_AndAssign,    5},
        {TokenType::T_XorAssign,    5},
        {TokenType::T_OrAssign,     5},
        {TokenType::T_LShiftAssign, 5},
        {TokenType::T_RShiftAssign, 5}
    };
    auto it = precedence.find(t);
    if (it != precedence.end()) {
        return it->second;
    }
    return -1;
}

void Parser::nextToken() {
    index++;
    currentToken = tokens[index];
}

Token Parser::peek(const int offset) {
    if (index+offset > tokens.size()) {
        Logger::Error("Token out of bounds.");
        exit(EXIT_FAILURE);
    }
    return tokens[index+offset];
}

void Parser::error(const std::string &message) {
    Logger::Report(currentToken, message);
    // skips all tokens until next ;
    do {
        nextToken();
    } while (currentToken.type != TokenType::T_Semicolon);
    eat(TokenType::T_Semicolon);
    cout<<tokenToString(currentToken.type)+" " + currentToken.value << endl;
}

unique_ptr<BlockAST> Parser::parse() {
    auto block = make_unique<BlockAST>(); // The file itself is a block
    while (currentToken.type != TokenType::T_EOF) {
        if (unique_ptr<AST> statement = parseStatement()) {
            block->statements.emplace_back(move(statement));
            if (dynamic_cast<FunctionDefinitionAST*>(block->statements.back().get()) == nullptr &&
                dynamic_cast<BlockAST*>(block->statements.back().get()) == nullptr &&
                dynamic_cast<StructDefinitionAST*>(block->statements.back().get()) == nullptr &&
                dynamic_cast<ExtendsStatementAST*>(block->statements.back().get()) == nullptr &&
                dynamic_cast<IfStatementAST*>(block->statements.back().get()) == nullptr &&
                dynamic_cast<ExternExprAST*>(block->statements.back().get()) == nullptr) {
                // If it's not a function definition, block, struct, or extends statement,
                // then it must be a statement, requires a semicolon
                eat(TokenType::T_Semicolon);
                }
        } else {
            Logger::Report(currentToken, "Unexpected token at top level: " + tokenToString(currentToken.type) + ":" + currentToken.value + ".");
            nextToken();
        }
        if (currentToken.type == TokenType::T_Semicolon)
            eat(TokenType::T_Semicolon);
    }
    return block;
}

unique_ptr<BlockAST> Parser::parseBlock() {
    eat(TokenType::T_LBrace);
    std::vector<unique_ptr<AST>> statements;
    while (currentToken.type != TokenType::T_RBrace && currentToken.type != TokenType::T_EOF) {

        if (unique_ptr<AST> statement = parseStatement()) {
            statements.emplace_back(move(statement));
            if (dynamic_cast<FunctionDefinitionAST*>(statements.back().get()) == nullptr &&
                dynamic_cast<BlockAST*>(statements.back().get()) == nullptr &&
                dynamic_cast<StructDefinitionAST*>(statements.back().get()) == nullptr &&
                dynamic_cast<ExtendsStatementAST*>(statements.back().get()) == nullptr &&
                dynamic_cast<ExternExprAST*>(statements.back().get()) == nullptr) {
                eat(TokenType::T_Semicolon);
                }
        } else {
            Logger::Report(currentToken, "Unexpected token inside block: " + tokenToString(currentToken.type));
            nextToken(); // Prevents infinite loop
        }

    }
    eat(TokenType::T_RBrace);
    return make_unique<BlockAST>(move(statements));
}

void Parser::eat(const TokenType type) {
    if (currentToken.type != type) {
        Logger::Report(currentToken, "Found '" + tokenToString(currentToken.type) +" "+currentToken.value+ "', expected '"+ tokenToString(type)+"'.");
        exit(EXIT_FAILURE);
    }
    nextToken();
}

unique_ptr<AST> Parser::parseStatement() {
    switch (currentToken.type) {
        case TokenType::T_Extern: {
            if (currentToken.value != "extern") {
                string val = currentToken.value;
                eat(TokenType::T_Extern);
                return make_unique<ExternExprAST>(val);
            }
            return parseExternStatement();
        }
        case TokenType::T_Constructor: return parseConstructorDefinition();
        case TokenType::T_Struct:  return parseStructDefinition();
        case TokenType::T_LBrace:  return parseBlock();
        case TokenType::T_Extends: return parseExtendsStatement();
        case TokenType::T_Return:  return parseReturn();
        case TokenType::T_If:      return parseIfStatement();
        case TokenType::T_Static:  return parseStaticDefinition();
        case TokenType::T_ID: {
            if (peek(1).type == TokenType::T_ID && peek(2).type == TokenType::T_LParen) {
                return parseFunctionDefinition(false);
            }
            if (peek(1).type == TokenType::T_ID && peek(2).type == TokenType::T_Assign) {
                return parseVariableDeclaration();
            }
            if (peek(1).type == TokenType::T_ID) { // variable declaration without initializer
                return parseVariableDeclaration();
            }
            if (peek(1).type == TokenType::T_Extends) { // Struct inheritance
                return parseExtendsStatement();
            }
            return parseExpr();
        }
        case TokenType::T_Int: return parseExpr();
        case TokenType::T_String: return parseString();
        default: {
            Logger::Error("Unexpected token in statement: " + tokenToString(currentToken.type));
            return nullptr;
        }
    }
}

unique_ptr<ExprAST> Parser::parseExpr() {
    auto LHS = parsePostfixExpr();
    if (!LHS)
        return nullptr;
    return parseOpRHS(0, move(LHS));
}

unique_ptr<ExprAST> Parser::parsePostfixExpr() {
    auto expr = parsePrimary();
    if (!expr)
        return nullptr;

    // If a '.' is found, then process the method call
    while (true) {
        if (currentToken.type == TokenType::T_Dot) {
            nextToken(); // Eat the dot

            if (currentToken.type != TokenType::T_ID) {
                Logger::Error("Expected method name after '.'.");
                return nullptr;
            }
            std::string accessName = currentToken.value;
            nextToken(); // eat the method name

            if (currentToken.type != TokenType::T_LParen) {
                expr = make_unique<FieldAccessAST>(move(expr), accessName);
                continue;
            }

            eat(TokenType::T_LParen);

            // Parse method args
            std::vector<unique_ptr<ExprAST>> args;
            if (currentToken.type != TokenType::T_RParen) {
                while (true) {
                    if (auto arg = parseExpr())
                        args.push_back(move(arg));
                    else
                        return nullptr;

                    if (currentToken.type == TokenType::T_RParen)
                        break;
                    eat(TokenType::T_Comma);
                }
            }
            eat(TokenType::T_RParen);

            expr = make_unique<MethodCallAST>(move(expr), accessName, move(args));
        } else {
            // Break if it's not a '.'
            break;
        }
    }
    return expr;
}

unique_ptr<ExprAST> Parser::parseOpRHS(const int exprPrecedence, unique_ptr<ExprAST> LHS) {
    while (true) {
        int precedence = getTokenPrecedence(currentToken.type);
        if (precedence < exprPrecedence)
            return LHS;

        auto op = currentToken.type;
        nextToken(); // eat the operator

        // If it's a simple assignment, create a specific AST node for it.
        if (op == TokenType::T_Assign) {
            auto RHS = parseExpr();
            if (!RHS) return nullptr;
            LHS = make_unique<VariableAssignmentAST>(move(LHS), move(RHS));
            continue;
        }

        auto RHS = parsePostfixExpr(); // Parse the right side of the current expression
        if (!RHS)
            return nullptr;

        if (const int nextPrecedence = getTokenPrecedence(currentToken.type); precedence < nextPrecedence) {
            RHS = parseOpRHS(precedence + 1, move(RHS));
            if (!RHS)
                return nullptr;
        }

        LHS = make_unique<OperationExprAST>(op, move(LHS), move(RHS));
    }
}

// Identifier - Number - String - (Expression)
unique_ptr<ExprAST> Parser::parsePrimary() {
    switch (currentToken.type) {
        case TokenType::T_ID:     return parseIdentifierExpr(); // VariableExprAST or CallExprAST
        case TokenType::T_Float:  return parseFloatExpr();
        case TokenType::T_Int:    return parseIntExpr();
        case TokenType::T_String: return parseString();
        case TokenType::T_LParen: return parseParenExpr();
        // Unary operators
        case TokenType::T_Not:
        case TokenType::T_Complement:
        case TokenType::T_Add: // Unary plus  (ex : int foo = +a)
        case TokenType::T_Sub: // Unary minus (ex : int foo = -a)
        case TokenType::T_Increment:
        case TokenType::T_Decrement:
            return parseUnaryExpr();
        case TokenType::T_Extern: {
            if (currentToken.value != "extern") {
                string val = currentToken.value;
                eat(TokenType::T_Extern);
                return make_unique<ExternExprAST>(val);
            }
        }
        default: {
            Logger::Error("Expected an expression, found '" + tokenToString(currentToken.type) + "'.");
            nextToken();
            return nullptr;
        }
    }
}

unique_ptr<ExprAST> Parser::parseUnaryExpr() {
    TokenType op = currentToken.type;
    nextToken(); // Eat the operator
    auto operand = parsePrimary();
    if (!operand) return nullptr;
    return make_unique<OperationExprAST>(op, nullptr, move(operand));
}

// TODO : add more number types
unique_ptr<FloatExprAST> Parser::parseFloatExpr() {
    auto res = make_unique<FloatExprAST>(stof(currentToken.value));
    eat(TokenType::T_Float);
    return move(res);
}

unique_ptr<IntExprAST> Parser::parseIntExpr() {
    auto res = make_unique<IntExprAST>(stoi(currentToken.value));
    eat(TokenType::T_Int);
    return move(res);
}

unique_ptr<StringExprAST> Parser::parseString() {
    string value = currentToken.value;
    eat(TokenType::T_String);
    return make_unique<StringExprAST>(value);
}

// (Expression) - ()
unique_ptr<ExprAST> Parser::parseParenExpr() {
    eat(TokenType::T_LParen);
    if (currentToken.type == TokenType::T_RParen) {
        eat(TokenType::T_RParen);
        return nullptr;
    }
    auto val = parseExpr();
    if (!val)
        return nullptr;
    eat(TokenType::T_RParen);
    return val;
}

// Identifier - Identifier(...)
unique_ptr<ExprAST> Parser::parseIdentifierExpr() {
    std::string name = currentToken.value;
    eat(TokenType::T_ID);

    if (currentToken.type == TokenType::T_LParen) { // Function call if LParen found
        eat(TokenType::T_LParen);
        std::vector<unique_ptr<ExprAST>> args;
        if (currentToken.type != TokenType::T_RParen) {
            while (true) {
                if (auto arg = parseExpr()) // Parses arguments
                    args.push_back(move(arg));
                else
                    return nullptr;

                if (currentToken.type == TokenType::T_RParen)
                    break;
                eat(TokenType::T_Comma);
            }
        }
        eat(TokenType::T_RParen);
        return make_unique<FunctionCallAST>(name, move(args));
    }
    // Else : simple variable
    return make_unique<VariableExprAST>(name);
}

// extern* - extern.foo - extern{foo, bar}   -   extern { c code here }
unique_ptr<ExternStatementAST> Parser::parseExternStatement() {
    eat(TokenType::T_Extern);

    if (currentToken.type != TokenType::T_ID) {
        Logger::Error("Expected library name after 'extern'.");
        return nullptr;
    }
    std::string libName = currentToken.value;
    eat(TokenType::T_ID);

    return make_unique<ExternStatementAST>(libName);
}

// TODO : type with generics parsing (Type<A,B>)
unique_ptr<TypeAST> Parser::parseType() {
    unique_ptr<TypeAST> type;
    if (currentToken.type == TokenType::T_ID && currentToken.value == "int") {
        type = make_unique<TypeAST>("int");
        eat(TokenType::T_ID);
        return type;
    } if (currentToken.type == TokenType::T_ID && currentToken.value == "string") {
        type = make_unique<TypeAST>("string");
        eat(TokenType::T_ID);
        return type;
    } if (currentToken.type == TokenType::T_ID) { // Custom type
        type = make_unique<TypeAST>(currentToken.value);
        eat(TokenType::T_ID);
        return type;
    }

    // Parse array : [type] or [type, size]
    if (currentToken.type == TokenType::T_LBracket) {
        eat(TokenType::T_LBracket);
        unique_ptr<ExprAST> arraySize = nullptr;
        if (currentToken.type != TokenType::T_RBracket) {
            // Check if there's an explicit size
            if (currentToken.type != TokenType::T_ID && currentToken.type != TokenType::T_Int) {
                Logger::Error("Expected type or size in array declaration.");
                return nullptr;
            }
            // TODO : Array parsing
        }
        eat(TokenType::T_RBracket);
        return make_unique<TypeAST>(std::move(type), std::move(arraySize));
    }
    Logger::Report(currentToken, "Expected a type.");
    return nullptr;
}

// type name
unique_ptr<FunctionParameterAST> Parser::parseFunctionParameter() {
    auto type = parseType();
    if (!type) return nullptr;
    // comment needed, because of stranger IDE warning
    // ReSharper disable once CppDFAConstantConditions
    if (currentToken.type != TokenType::T_ID) {
        Logger::Error("Expected parameter name.");
        return nullptr;
    }
    // ReSharper disable once CppDFAUnreachableCode
    std::string name = currentToken.value;
    eat(TokenType::T_ID);
    return make_unique<FunctionParameterAST>(std::move(type), name);
}

// type function(params) {} - type function(params) = ...;
unique_ptr<FunctionDefinitionAST> Parser::parseFunctionDefinition(bool isStatic=false) {
    auto returnType = parseType();
    if (!returnType) return nullptr;

    if (currentToken.type != TokenType::T_ID) {
        Logger::Error("Expected function name.");
        return nullptr;
    }
    std::string funcName = currentToken.value;
    eat(TokenType::T_ID);

    eat(TokenType::T_LParen);
    std::vector<unique_ptr<FunctionParameterAST>> params;
    if (currentToken.type != TokenType::T_RParen) {
        while (true) {
            if (auto param = parseFunctionParameter()) {
                params.push_back(std::move(param));
            } else {
                return nullptr;
            }
            if (currentToken.type == TokenType::T_RParen) break;
            eat(TokenType::T_Comma);
        }
    }
    eat(TokenType::T_RParen);

    unique_ptr<AST> body = nullptr;
    if (currentToken.type == TokenType::T_LBrace) {
        body = parseBlock(); // A block for multi-statement functions
    } else if (currentToken.type == TokenType::T_Assign) {
        eat(TokenType::T_Assign);
        body = parseExpr(); // A single expression for ' = ' functions
    } else {
        Logger::Error("Expected '{' or '=' for function body.");
        return nullptr;
    }
    return make_unique<FunctionDefinitionAST>(move(returnType), funcName, move(params), move(body), isStatic);
}

// function(params...)
unique_ptr<FunctionCallAST> Parser::parseFunctionCall() {
    std::string funcName = currentToken.value;
    eat(TokenType::T_ID);

    eat(TokenType::T_LParen);
    std::vector<unique_ptr<ExprAST>> params;
    if (currentToken.type != TokenType::T_RParen) {
        while (true) {
            if (auto param = parseExpr())
                params.push_back(std::move(param));
            else
                return nullptr;
            if (currentToken.type == TokenType::T_RParen) break;
            eat(TokenType::T_Comma);
        }
    }
    eat(TokenType::T_RParen);
    return make_unique<FunctionCallAST>(funcName, std::move(params));
}



// type var - type var = ...
unique_ptr<VariableDeclarationAST> Parser::parseVariableDeclaration() {
    unique_ptr<TypeAST> varType = parseType();
    if (!varType)
        return nullptr;

    if (currentToken.type != TokenType::T_ID) {
        Logger::Error("Expected variable name.");
        return nullptr;
    }
    std::string varName = currentToken.value;
    eat(TokenType::T_ID);

    unique_ptr<ExprAST> initializer = nullptr;
    if (currentToken.type == TokenType::T_Assign) {
        eat(TokenType::T_Assign);
        initializer = parseExpr();
        if (!initializer) return nullptr;
    }
    return make_unique<VariableDeclarationAST>(std::move(varType), varName, std::move(initializer));
}

// struct name {fields...} - struct name<A,B> {fields...}
unique_ptr<StructDefinitionAST> Parser::parseStructDefinition() {
    eat(TokenType::T_Struct);
    if (currentToken.type != TokenType::T_ID) {
        Logger::Error("Expected struct name.");
        return nullptr;
    }
    std::string structName = currentToken.value;
    eat(TokenType::T_ID);

    std::vector<unique_ptr<GenericParameterAST>> genericParams;
    if (currentToken.type == TokenType::T_LT) { // Generics
        eat(TokenType::T_LT);
        while (currentToken.type != TokenType::T_GT) {
            if (currentToken.type != TokenType::T_ID) {
                Logger::Error("Expected generic parameter name.");
                return nullptr;
            }
            genericParams.push_back(make_unique<GenericParameterAST>(currentToken.value));
            eat(TokenType::T_ID);
            if (currentToken.type == TokenType::T_Comma) {
                eat(TokenType::T_Comma);
            } else if (currentToken.type != TokenType::T_GT) {
                Logger::Error("Expected ',' or '>' in generic parameter list.");
                return nullptr;
            }
        }
        eat(TokenType::T_GT);
    }

    eat(TokenType::T_LBrace);
    std::vector<unique_ptr<StructFieldAST>> fields;
    while (currentToken.type != TokenType::T_RBrace) {
        auto fieldType = parseType();
        if (!fieldType) return nullptr;

        // Comment needed because of strange IDE warning
        // ReSharper disable once CppDFAConstantConditions
        if (currentToken.type != TokenType::T_ID) {
            Logger::Error("Expected field name.");
            return nullptr;
        }
        // ReSharper disable once CppDFAUnreachableCode
        std::string fieldName = currentToken.value;
        eat(TokenType::T_ID);
        eat(TokenType::T_Semicolon); // Each field declaration ends with a semicolon
        fields.push_back(make_unique<StructFieldAST>(std::move(fieldType), fieldName));
    }
    eat(TokenType::T_RBrace);
    return make_unique<StructDefinitionAST>(structName, std::move(genericParams), std::move(fields));
}

// Same as parseFunctionDefinition but with "constructor" keyword
unique_ptr<ConstructorDefinitionAST> Parser::parseConstructorDefinition() {
    eat(TokenType::T_Constructor);
    eat(TokenType::T_LParen);
    std::vector<unique_ptr<FunctionParameterAST>> params;
    if (currentToken.type != TokenType::T_RParen) {
        while (true) {
            if (auto param = parseFunctionParameter()) {
                params.push_back(std::move(param));
            } else {
                return nullptr;
            }
            if (currentToken.type == TokenType::T_RParen) break;
            eat(TokenType::T_Comma);
        }
    }
    eat(TokenType::T_RParen);

    unique_ptr<AST> body;
    if (currentToken.type == TokenType::T_LBrace) {
        body = parseBlock(); // A block for multi-statement functions
    } else if (currentToken.type == TokenType::T_Assign) {
        eat(TokenType::T_Assign);
        body = parseExpr(); // A single expression for ' = ' functions
    } else {
        Logger::Error("Expected '{' or '=' for function body.");
        return nullptr;
    }
    // Struct name will be defined during analysis
    return make_unique<ConstructorDefinitionAST>("", std::move(params), std::move(body));
}

// extends SomeStruct{} - childStruct extends SomeStruct {}
unique_ptr<ExtendsStatementAST> Parser::parseExtendsStatement() {
    if (currentToken.type == TokenType::T_ID) { // inheritance case (B extends A {})
        std::string childName = currentToken.value;
        eat(TokenType::T_ID);
        if (currentToken.type != TokenType::T_Extends) {
            Logger::Error("Expected 'extends' after child struct name.");
            return nullptr;
        }
        eat(TokenType::T_Extends);
        if (currentToken.type != TokenType::T_ID) {
            Logger::Error("Expected parent struct name after 'extends'.");
            return nullptr;
        }
        std::string parentName = currentToken.value;
        eat(TokenType::T_ID);
        const auto body = parseBlock();
        return make_unique<ExtendsStatementAST>(childName, parentName, move(body->statements));
    }

    // else: simple struct extending
    eat(TokenType::T_Extends);
    if (currentToken.type != TokenType::T_ID) {
        Logger::Error("Expected struct name to extend.");
        return nullptr;
    }
    std::string structName = currentToken.value;
    eat(TokenType::T_ID);

    const auto body = parseBlock();
    return make_unique<ExtendsStatementAST>(structName, std::move(body->statements));
}

// return Expression
unique_ptr<ReturnAST> Parser::parseReturn() {
    eat(TokenType::T_Return);
    auto ret = parseExpr();
    if (!ret) return nullptr;
    return make_unique<ReturnAST>(move(ret));
}

// if (condition) {} - if (condition) {} else ...
unique_ptr<IfStatementAST> Parser::parseIfStatement() {
    eat(TokenType::T_If);
    unique_ptr<ExprAST> condition = parseParenExpr();
    if (!condition) {
        error("If statement expected a condition.");
        return nullptr;
    }

    unique_ptr<AST> thenBody;
    if (currentToken.type == TokenType::T_LBrace) {
        thenBody = parseBlock();
    } else {
        thenBody = parseStatement();
        if (dynamic_cast<FunctionDefinitionAST*>(thenBody.get()) == nullptr &&
            dynamic_cast<BlockAST*>(thenBody.get()) == nullptr &&
            dynamic_cast<StructDefinitionAST*>(thenBody.get()) == nullptr &&
            dynamic_cast<ExtendsStatementAST*>(thenBody.get()) == nullptr) {
            eat(TokenType::T_Semicolon);
            }
    }
    if (!thenBody) {
        error("Expected body after 'if' condition.");
        return nullptr;
    }
    if (currentToken.type == TokenType::T_Else) {
        eat(TokenType::T_Else);
        unique_ptr<AST> elseBody;
        if (currentToken.type == TokenType::T_LBrace) {
            elseBody = parseBlock();
        } else {
            elseBody = parseStatement();
            if (dynamic_cast<FunctionDefinitionAST*>(elseBody.get()) == nullptr &&
                dynamic_cast<BlockAST*>(elseBody.get()) == nullptr &&
                dynamic_cast<StructDefinitionAST*>(elseBody.get()) == nullptr &&
                dynamic_cast<ExtendsStatementAST*>(elseBody.get()) == nullptr) {
                eat(TokenType::T_Semicolon);
                }
        }
        if (!elseBody) {
            Logger::Error("Expected body after 'else'.");
            return nullptr;
        }
        return make_unique<IfStatementAST>(move(condition), move(thenBody), move(elseBody));
    }
    return make_unique<IfStatementAST>(move(condition), move(thenBody));
}

unique_ptr<FunctionDefinitionAST> Parser::parseStaticDefinition() {
    eat(TokenType::T_Static);
    return parseFunctionDefinition(true);
}