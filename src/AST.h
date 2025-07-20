#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Lexer.h"
#include "SymbolTable.h"

using namespace std;

class AST {
public:
    virtual ~AST() = default;
    virtual void analyse(SymbolTable& table) {}
    virtual void prePass(SymbolTable& table) {}
    virtual string code() {return "";}
    // Ajout de la méthode de clonage virtuelle pure
    [[nodiscard]] virtual unique_ptr<AST> clone() const = 0;
};

class BlockAST final : public AST {
public:
    std::vector<std::unique_ptr<AST>> statements;
    explicit BlockAST(std::vector<std::unique_ptr<AST>> s) : statements(std::move(s)) {}
    BlockAST() = default;
    void prePass(SymbolTable& table) override;
    void analyse(SymbolTable& table) override;
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class ExprAST : public AST {
public:
    void prePass(SymbolTable& table) override;
    virtual void analyse(SymbolTable& table, string& a) {}
    string code() override { return ""; }
};

class FloatExprAST final : public ExprAST {
    float val;
public:
    void analyse(SymbolTable& table, string& a) override;
    explicit FloatExprAST(const float val) : val(val) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class IntExprAST final : public ExprAST {
    int val;
public:
    void analyse(SymbolTable& table, string& a) override;
    explicit IntExprAST(const int val) : val(val) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class StringExprAST final : public ExprAST {
    string val;
public:
    void analyse(SymbolTable& table, string& a) override;
    explicit StringExprAST(string val) : val(std::move(val)) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class VariableExprAST : public ExprAST {
public:
    bool isField;
    std::string name;

    void analyse(SymbolTable& table, string& a) override;
    explicit VariableExprAST(std::string name) : isField(false), name(std::move(name)) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class OperationExprAST final : public ExprAST {
    TokenType op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:
    void analyse(SymbolTable& table, string& a) override;
    OperationExprAST(const TokenType op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS) :
        op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class ExternStatementAST final : public AST {
public:
    std::string libraryName;
    explicit ExternStatementAST(std::string libName) : libraryName(std::move(libName)) {}
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class TypeAST final : public AST {
public:
    std::string type;
    std::vector<unique_ptr<TypeAST>> genericArgs;
    bool isArray = false;
    std::unique_ptr<ExprAST> arraySize; // Optional: for fixed-size arrays

    void analyse(SymbolTable &table) override;

    explicit TypeAST(string t) : type(std::move(t)) {}
    TypeAST(string base, vector<unique_ptr<TypeAST>> args) : type(std::move(base)), genericArgs(std::move(args)) {}
    explicit TypeAST(const unique_ptr<TypeAST> &type, std::unique_ptr<ExprAST> size) : type(type->type), arraySize(move(size)) {}

    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class FunctionParameterAST final : public AST {
public:
    std::unique_ptr<TypeAST> type;
    std::string name;
    FunctionParameterAST(std::unique_ptr<TypeAST> t, std::string n) : type(std::move(t)), name(std::move(n)) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class FunctionDefinitionAST final : public AST {
public:
    std::unique_ptr<TypeAST> returnType;
    bool isStatic = false;
    std::string name;
    std::vector<std::unique_ptr<FunctionParameterAST>> params;
    std::unique_ptr<AST> body;

    void prePass(SymbolTable& table) override;
    void analyse(SymbolTable &table) override;
    void analyse(SymbolTable &table, const string& parentStruct);
    FunctionDefinitionAST(std::unique_ptr<TypeAST> retType, std::string n, std::vector<unique_ptr<FunctionParameterAST>> p, std::unique_ptr<AST> b, const bool isStatic = false)
        : returnType(std::move(retType)), isStatic(isStatic), name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
    string code(bool isMethod);
    string code() override;
    string getSignature();
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class FunctionCallAST : public ExprAST {
public:
    std::string name;
    string signature;
    std::vector<unique_ptr<ExprAST>> params;

    void analyse(SymbolTable& table, string& a) override;
    FunctionCallAST(string name, vector<unique_ptr<ExprAST>> params) :
        name(std::move(name)), params(move(params)) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class MethodCallAST final : public FunctionCallAST {
public:
    unique_ptr<ExprAST> ownerExpr;
    void analyse(SymbolTable& table, string& a) override;
    string code() override;
    MethodCallAST(unique_ptr<ExprAST> owner, string name, vector<unique_ptr<ExprAST>> params) :
        FunctionCallAST(move(name), move(params)), ownerExpr(move(owner)) {}
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class FieldAccessAST final : public VariableExprAST {
    unique_ptr<ExprAST> ownerExpr;
public:
    void analyse(SymbolTable &table, string &a) override;
    string code() override;
    FieldAccessAST(unique_ptr<ExprAST> owner, string name) : VariableExprAST(move(name)), ownerExpr(move(owner)) {}
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class VariableDeclarationAST final : public AST {
public:
    std::unique_ptr<TypeAST> type;
    std::string name;
    std::unique_ptr<ExprAST> initializer;

    void analyse(SymbolTable& table) override;
    VariableDeclarationAST(std::unique_ptr<TypeAST> t, std::string n, std::unique_ptr<ExprAST> init)
        : type(std::move(t)), name(std::move(n)), initializer(std::move(init)) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class VariableAssignmentAST final : public ExprAST {
public:
    unique_ptr<ExprAST> target;
    unique_ptr<ExprAST> value;
    string accessor;

    void analyse(SymbolTable& table) override;
    void analyse(SymbolTable& table, string& a) override;
    string code() override;
    VariableAssignmentAST(unique_ptr<ExprAST> target, unique_ptr<ExprAST> val)
        : target(std::move(target)), value(std::move(val)) {}
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class GenericParameterAST final : public AST {
public:
    std::string name;
    explicit GenericParameterAST(std::string n) : name(std::move(n)) {}
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class StructFieldAST final : public AST {
public:
    std::unique_ptr<TypeAST> type;
    std::string name;
    StructFieldAST(unique_ptr<TypeAST> t, string n) : type(move(t)), name(move(n)) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class StructDefinitionAST final : public AST {
public:
    std::string name;
    std::vector<std::unique_ptr<GenericParameterAST>> genericParams;
    std::vector<std::unique_ptr<StructFieldAST>> fields;

    void prePass(SymbolTable& table) override;
    void analyse(SymbolTable& table) override;
    StructDefinitionAST(std::string n, std::vector<unique_ptr<GenericParameterAST>> g, std::vector<unique_ptr<StructFieldAST>> f)
        : name(std::move(n)), genericParams(std::move(g)), fields(std::move(f)) {}
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class ConstructorDefinitionAST final : public AST {
public:
    string structName;
    std::vector<std::unique_ptr<FunctionParameterAST>> params;
    std::unique_ptr<AST> body;

    void prePass(SymbolTable& table) override;
    void analyse(SymbolTable& table) override;
    string code() override;
    string getSignature();
    ConstructorDefinitionAST(string structName, std::vector<unique_ptr<FunctionParameterAST>> p, unique_ptr<AST> b)
        : structName(std::move(structName)), params(std::move(p)), body(std::move(b)) {}
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class ExtendsStatementAST final : public AST {
public:
    std::string structName;
    std::string parentStructName;
    std::vector<std::unique_ptr<AST>> members;

    void analyse(SymbolTable& table) override;
    ExtendsStatementAST(std::string name, std::vector<unique_ptr<AST>> m) : structName(std::move(name)), members(std::move(m)) {}
    ExtendsStatementAST(std::string childName, std::string parentName, vector<unique_ptr<AST>> members = {}) :
        structName(std::move(childName)), parentStructName(std::move(parentName)), members(move(members)) {}
    bool isFieldOnly();
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class ReturnAST final : public AST {
public:
    unique_ptr<ExprAST> value;
    explicit ReturnAST(unique_ptr<ExprAST> value) : value(move(value)) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class IfStatementAST final : public AST {
public:
    unique_ptr<ExprAST> condition;
    unique_ptr<AST> thenBody;
    unique_ptr<AST> elseBody;
    IfStatementAST(unique_ptr<ExprAST> condition, unique_ptr<AST> a, unique_ptr<AST> b = nullptr) :
        condition(move(condition)), thenBody(move(a)), elseBody(move(b)) {}
    [[nodiscard]] unique_ptr<AST> clone() const override;
};

class ExternExprAST final : public ExprAST {
public:
    string body;
    explicit ExternExprAST(string body) : body(std::move(body)) {}
    string code() override;
    [[nodiscard]] unique_ptr<AST> clone() const override;
};