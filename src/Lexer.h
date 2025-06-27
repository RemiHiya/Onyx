//
// Created by remsc on 05/06/2025.
//

#ifndef LEXER_H
#define LEXER_H
#include <map>
#include <string>
#include <vector>

enum class TokenType {
    T_ID,
    T_Comma,      // ,
    T_Semicolon,  // ;
    T_Dot,        // .
    T_StaticCall, // ::
    T_LParen,     // (
    T_RParen,     // )
    T_LBrace,     // {
    T_RBrace,     // }
    T_LBracket,   // [
    T_RBracket,   // ]
    // Types
    T_Type,
    T_Int,
    T_Float,
    T_Bool,
    T_Struct,
    T_Constructor,
    T_String,
    T_Char,
    // Keywords
    T_Extern,
    T_Extends,
    T_Return,
    T_For,
    T_While,
    T_Continue,
    T_Break,
    T_If,
    T_Else,
    T_Static,
    // Operators
    T_Assign,       // =
    T_Increment,    // ++
    T_Decrement,    // --
    T_AddAssign,    // +=
    T_SubAssign,    // -=
    T_MulAssign,    // *=
    T_DivAssign,    // /=
    T_ModAssign,    // %=
    T_AndAssign,    // &=
    T_XorAssign,    // ^=
    T_OrAssign,     // |=
    T_LShiftAssign, // <<=
    T_RShiftAssign, // >>=
    T_Mul,          // *
    T_Div,          // /
    T_Mod,          // %
    T_Add,          // +
    T_Sub,          // -
    T_Not,          // !
    T_Complement,   // ~
    T_LBitShift,    // <<
    T_RBitShift,    // >>
    T_BitAND,       // &
    T_BitOR,        // |
    T_BitXOR,       // ^
    T_LogOR,        // ||
    T_LogAND,       // &&
    T_GT,           // >
    T_GE,           // >=
    T_LT,           // <
    T_LE,           // <=
    T_Equals,       // ==
    T_NotEquals,    // !=
    // Misc
    T_EOF,
    T_Error,
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;
};

std::string tokenToString(const TokenType &token);

class Lexer {
    std::string source;
    size_t current_pos = 0;
    int current_line = 1;
    int current_column = 1;

    std::map<std::string, TokenType> keywords;
    void initKeywords();

    char peek(int offset = 0) const;
    char advance();
    void skipWhitespace();
    Token createToken(TokenType type, const std::string& value = "", int startCol = -1) const;
    Token matchKeywordOrIdentifier(int startCol);
    Token matchNumber(int startCol);
    Token matchString(int startCol);

public:
    explicit Lexer(std::string in);
    std::vector<Token> tokenize();
};

#endif //LEXER_H
