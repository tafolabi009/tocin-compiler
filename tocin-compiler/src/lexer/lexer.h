// tocin-compiler/src/lexer/lexer.h
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "token.h"

class Lexer {
public:
    Lexer(const std::string& source, const std::string& filename);
    std::vector<Token> tokenize();

private:
    std::string source;
    std::string filename;
    size_t position = 0;
    size_t line = 1;
    size_t column = 1;
    int indentLevel = 0;
    bool atLineStart = true;

    char peek() const;
    char advance();
    bool match(char expected);
    void skipWhitespace();
    void handleIndentation();

    Token makeToken(TokenType type, const std::string& value = "");
    Token scanIdentifier();
    Token scanNumber();
    Token scanString();

    static std::unordered_map<std::string, TokenType> keywords;
};

// tocin-compiler/src/lexer/token.h
#pragma once

#include <string>

enum class TokenType {
    // Keywords
    DEF, CLASS, IF, ELIF, ELSE, FOR, IN, WHILE, RETURN, IMPORT, FROM,
    MATCH, CASE, DEFAULT, ASYNC, AWAIT, CONST, LET, UNSAFE, INTERFACE,
    OVERRIDE, SPAWN, PURE,

    // Types
    TYPE, INT, INT8, INT16, INT32, INT64, UINT, UINT8, UINT16, UINT32, UINT64,
    FLOAT32, FLOAT64, BOOL, CHAR, STRING, LIST, MAP, SET, TUPLE, OPTION, RESULT,

    // Literals
    IDENTIFIER, INTEGER_LITERAL, FLOAT_LITERAL, STRING_LITERAL, BOOL_LITERAL,

    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT, EQUAL, PLUS_EQUAL, MINUS_EQUAL,
    STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL, BANG, BANG_EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL, LESS, LESS_EQUAL, AND, OR, ARROW, COLON,
    DOUBLE_COLON, DOT, COMMA, QUESTION, AT, HASH,

    // Delimiters
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACKET, RIGHT_BRACKET, LEFT_BRACE, RIGHT_BRACE,

    // Indentation
    INDENT, DEDENT, NEWLINE,

    // Special
    EOF_TOKEN, ERROR
};

struct Token {
    TokenType type;
    std::string value;
    std::string filename;
    size_t line;
    size_t column;

    Token(TokenType type, const std::string& value, const std::string& filename,
        size_t line, size_t column)
        : type(type), value(value), filename(filename), line(line), column(column) {}

    std::string toString() const;
};