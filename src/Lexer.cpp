//
// Created by remsc on 05/06/2025.
//

#include "Lexer.h"

#include <iostream>
#include <utility>

#include "Logger.h"

std::string tokenToString(const TokenType& token) {
    switch (token) {
        case TokenType::T_ID: return "ID";
        case TokenType::T_Comma: return ",";
        case TokenType::T_Semicolon: return ";";
        case TokenType::T_Dot: return ".";
        case TokenType::T_Int: return "Int";
        case TokenType::T_String: return "String";

        case TokenType::T_Assign: return "=";
        case TokenType::T_Increment: return "++";
        case TokenType::T_Decrement: return "--";
        case TokenType::T_AddAssign: return "+=";
        case TokenType::T_SubAssign: return "-=";
        case TokenType::T_MulAssign: return "*=";
        case TokenType::T_DivAssign: return "/=";
        case TokenType::T_ModAssign: return "%=";
        case TokenType::T_AndAssign: return "&=";
        case TokenType::T_XorAssign: return "^=";
        case TokenType::T_OrAssign: return "|=";
        case TokenType::T_LShiftAssign: return "<<=";
        case TokenType::T_RShiftAssign: return ">>=";

        case TokenType::T_Mul: return "*";
        case TokenType::T_Div: return "/";
        case TokenType::T_Mod: return "%";
        case TokenType::T_Add: return "+";
        case TokenType::T_Sub: return "-";

        case TokenType::T_Not: return "!";
        case TokenType::T_Complement: return "~";
        case TokenType::T_LBitShift: return "<<";
        case TokenType::T_RBitShift: return ">>";
        case TokenType::T_BitAND: return "&";
        case TokenType::T_BitOR: return "|";
        case TokenType::T_BitXOR: return "^";
        case TokenType::T_LogOR: return "||";
        case TokenType::T_LogAND: return "&&";

        case TokenType::T_GT: return ">";
        case TokenType::T_GE: return ">=";
        case TokenType::T_LT: return "<";
        case TokenType::T_LE: return "<=";
        case TokenType::T_Equals: return "==";
        case TokenType::T_NotEquals: return "!=";

        case TokenType::T_LParen: return "(";
        case TokenType::T_RParen: return ")";
        case TokenType::T_LBrace: return "{";
        case TokenType::T_RBrace: return "}";
        case TokenType::T_LBracket: return "[";
        case TokenType::T_RBracket: return "]";

        case TokenType::T_EOF: return "EOF";
        default: return "ERROR";
    }
}

void Lexer::initKeywords() {
    keywords["extern"] = TokenType::T_Extern;
    keywords["struct"] = TokenType::T_Struct;
    keywords["extends"] = TokenType::T_Extends;
    keywords["constructor"] = TokenType::T_Constructor;
    keywords["return"] = TokenType::T_Return;
    keywords["for"] = TokenType::T_For;
    keywords["while"] = TokenType::T_While;
    keywords["break"] = TokenType::T_Break;
    keywords["continue"] = TokenType::T_Continue;
    keywords["if"] = TokenType::T_If;
    keywords["else"] = TokenType::T_Else;
    keywords["static"] = TokenType::T_Static;
}

char Lexer::peek(const int offset) const {
    if (current_pos + offset >= source.length()) {
        return '\0';
    }
    return source[current_pos + offset];
}

char Lexer::advance() {
    if (current_pos >= source.length()) {
        return '\0';
    }
    const char c = source[current_pos++];
    if (c == '\n') {
        current_line++;
        current_column = 1;
    } else {
        current_column++;
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (current_pos < source.length() && std::isspace(peek())) {
        advance();
    }
}

Token Lexer::createToken(const TokenType type, const std::string& value, const int startCol) const {
    return {type, value, current_line, (startCol != -1) ? startCol : current_column - static_cast<int>(value.length())};
}

Token Lexer::matchKeywordOrIdentifier(const int startCol) {
    std::string value;
    if (std::isalpha(peek())) {
        value += advance();
        while (std::isalnum(peek()) || peek() == '_') {
            value += advance();
        }
    }
    // Check if it's a keyword
    if (keywords.contains(value)) {
        return createToken(keywords[value], value, startCol);
    }
    return createToken(TokenType::T_ID, value, startCol);
}

Token Lexer::matchNumber(const int startCol) {
    std::string value;
    bool isFloat = false;
    while (std::isdigit(peek())) {
        value += advance();
    }
    if (peek() == '.') {
        isFloat = true;
        value += advance();
        while (std::isdigit(peek())) {
            value += advance();
        }
    }
    return createToken(isFloat ? TokenType::T_Float :TokenType::T_Int, value, startCol);
}

Token Lexer::matchString(int startCol) {
    std::string value;
    advance(); // eat the first "
    while (peek() != '"' && peek() != '\0') {
        value += advance();
    }
    if (peek() == '\0') {
        std::cerr << "Error: Unclosed string literal at line " << current_line << ", column " << startCol << std::endl;
        return createToken(TokenType::T_EOF, value, startCol);
    }
    advance(); // eat the last "
    return createToken(TokenType::T_String, value, startCol);
}

Lexer::Lexer(std::string in) {
    source = std::move(in);
    initKeywords();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> out;
    Token current;

    while (current_pos < source.length()) {
        skipWhitespace();
        if (current_pos >= source.length()) {
            break;
        }

        int tokenStartCol = current_column;
        char c = peek();

        // Identifiers, strings and numbers
        if (std::isalpha(c) || c == '_') {
            current = matchKeywordOrIdentifier(tokenStartCol);
        } else if (c == '"') {
            current = matchString(tokenStartCol);
        } else if (std::isdigit(c) || (c == '.' && std::isdigit(peek(1)))) {   // supports float declaration like : .5 (instead of 0.5)
            current = matchNumber(tokenStartCol);
        }

        // Operators and Delimiters
        else {
            switch (c) {
                case ':': {
                    advance();
                    if (peek() == ':') {
                        advance();
                        current = createToken(TokenType::T_StaticCall, "::", tokenStartCol);
                    }
                    break;
                }
                case '=': {
                    advance();
                    if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_Equals, "==", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_Assign, "=", tokenStartCol);
                    }
                    break;
                }
                case '+': {
                    advance();
                    if (peek() == '+') {
                        advance();
                        current = createToken(TokenType::T_Increment, "++", tokenStartCol);
                    } else if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_AddAssign, "+=", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_Add, "+", tokenStartCol);
                    }
                    break;
                }
                case '-': {
                    advance();
                    if (peek() == '-') {
                        advance();
                        current = createToken(TokenType::T_Decrement, "--", tokenStartCol);
                    } else if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_SubAssign, "-=", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_Sub, "-", tokenStartCol);
                    }
                    break;
                }
                case '*': {
                    advance();
                    if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_MulAssign, "*=", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_Mul, "*", tokenStartCol);
                    }
                    break;
                }
                case '/': {
                    advance();
                    if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_DivAssign, "/=", tokenStartCol);
                    } else if (peek() == '/') {
                        do {
                            advance();
                        } while (peek() != '\n' && current_pos < source.length());
                        continue;
                    } else if (peek() == '*') {
                        advance(); // eat '*'
                        // Search for '*/'
                        while (current_pos < source.length()) {
                            if (peek() == '*' && peek(1) == '/') {
                                advance(); // eat '*'
                                advance(); // eat '/'
                                break;
                            }
                            advance();
                        }
                        continue;
                    } else {
                        current = createToken(TokenType::T_Div, "/", tokenStartCol);
                    }
                    break;
                }
                case '%': {
                    advance();
                    if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_ModAssign, "%=", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_Mod, "%", tokenStartCol);
                    }
                    break;
                }
                case '&': {
                    advance();
                    if (peek() == '&') {
                        advance();
                        current = createToken(TokenType::T_LogAND, "&&", tokenStartCol);
                    } else if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_AndAssign, "&=", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_BitAND, "&", tokenStartCol);
                    }
                    break;
                }
                case '|': {
                    advance();
                    if (peek() == '|') {
                        advance();
                        current = createToken(TokenType::T_LogOR, "||", tokenStartCol);
                    } else if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_OrAssign, "|=", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_BitOR, "|", tokenStartCol);
                    }
                    break;
                }
                case '^': {
                    advance();
                    if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_XorAssign, "^=", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_BitXOR, "^", tokenStartCol);
                    }
                    break;
                }
                case '!': {
                    advance();
                    if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_NotEquals, "!=", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_Not, "!", tokenStartCol);
                    }
                    break;
                }
                case '~': {
                    advance();
                    current = createToken(TokenType::T_Complement, "~", tokenStartCol);
                    break;
                }
                case '<': {
                    advance();
                    if (peek() == '<') {
                        advance();
                        if (peek() == '=') {
                            advance();
                            current = createToken(TokenType::T_LShiftAssign, "<<=", tokenStartCol);
                        } else {
                            current = createToken(TokenType::T_LBitShift, "<<", tokenStartCol);
                        }
                    } else if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_LE, "<=", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_LT, "<", tokenStartCol);
                    }
                    break;
                }
                case '>': {
                    advance();
                    if (peek() == '>') {
                        advance();
                        if (peek() == '=') {
                            advance();
                            current = createToken(TokenType::T_RShiftAssign, ">>=", tokenStartCol);
                        } else {
                            current = createToken(TokenType::T_RBitShift, ">>", tokenStartCol);
                        }
                    } else if (peek() == '=') {
                        advance();
                        current = createToken(TokenType::T_GE, ">=", tokenStartCol);
                    } else {
                        current = createToken(TokenType::T_GT, ">", tokenStartCol);
                    }
                    break;
                }
                case '(': advance(); current = createToken(TokenType::T_LParen, "(", tokenStartCol); break;
                case ')': advance(); current = createToken(TokenType::T_RParen, ")", tokenStartCol); break;
                case '{': advance(); current = createToken(TokenType::T_LBrace, "{", tokenStartCol); break;
                case '}': advance(); current = createToken(TokenType::T_RBrace, "}", tokenStartCol); break;
                case '[': advance(); current = createToken(TokenType::T_LBracket, "[", tokenStartCol); break;
                case ']': advance(); current = createToken(TokenType::T_RBracket, "]", tokenStartCol); break;
                case ';': advance(); current = createToken(TokenType::T_Semicolon, ";", tokenStartCol); break;
                case '.': advance(); current = createToken(TokenType::T_Dot, ".", tokenStartCol); break;
                case ',': advance(); current = createToken(TokenType::T_Comma, ",", tokenStartCol); break;
                default:
                    std::cerr << "Unexpected character found: '" << c << "' at line " << current_line << ", column " << current_column << std::endl;
                    advance();
                    current = createToken(TokenType::T_Error, std::string(1, c), tokenStartCol);
                    break;
            }
        }

        // FFI case
        if (current.type == TokenType::T_Extern) {
            skipWhitespace();
            if (peek(0) == '{') {
                advance();
                unsigned int counter = 1;
                std::string ffi;
                while (counter > 0) {
                    ffi += advance();
                    if (peek(0) == '}') counter--;
                    if (peek(0) == '{') counter++;
                    if (peek(0) == '\0') {
                        Logger::Error("Expected end of 'extern' block.");
                        exit(EXIT_FAILURE);
                    }
                }
                advance();
                out.push_back(createToken(TokenType::T_Extern, ffi, current_column));
                continue;
            }
        }
        // Push token if no error
        if (current.type != TokenType::T_Error) {
            out.push_back(current);
        }
    }
    out.push_back(createToken(TokenType::T_EOF, "EOF", current_column));
    return out;
}
