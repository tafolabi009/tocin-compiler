#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
#include <string>

enum class TokenType {
    // Keywords
    DEF,
    CLASS,
    IF,
    ELIF,
    ELSE,
    FOR,
    IN,
    WHILE,
    RETURN,
    IMPORT,
    FROM,
    MATCH,
    CASE,
    DEFAULT,
    ASYNC,
    AWAIT,
    CONST,
    LET,
    UNSAFE,
    INTERFACE,
    OVERRIDE,
    SPAWN,
    PURE,
    TRUE,
    FALSE,
    NIL,

    // Types
    TYPE,
    INT,
    INT8,
    INT16,
    INT32,
    INT64,
    UINT,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    FLOAT32,
    FLOAT64,
    BOOL,
    CHAR,
    STRING,
    LIST,
    MAP,
    SET,
    TUPLE,
    OPTION,
    RESULT,

    // Literals
    IDENTIFIER,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    BOOL_LITERAL,

    // Operators
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    PERCENT,        // %
    EQUAL,          // =
    PLUS_EQUAL,     // +=
    MINUS_EQUAL,    // -=
    STAR_EQUAL,     // *=
    SLASH_EQUAL,    // /=
    PERCENT_EQUAL,  // %=
    BANG,           // !
    BANG_EQUAL,     // !=
    EQUAL_EQUAL,    // ==
    GREATER,        // >
    GREATER_EQUAL,  // >=
    LESS,           // <
    LESS_EQUAL,     // <=
    AND,            // and
    OR,             // or
    ARROW,          // ->
    COLON,          // :
    DOUBLE_COLON,   // ::
    DOT,            // .
    COMMA,          // ,
    QUESTION,       // ?
    AT,             // @
    HASH,           // #

    // Delimiters
    LEFT_PAREN,     // (
    RIGHT_PAREN,    // )
    LEFT_BRACKET,   // [
    RIGHT_BRACKET,  // ]
    LEFT_BRACE,     // {
    RIGHT_BRACE,    // }

    // Indentation
    INDENT,
    DEDENT,
    NEWLINE,

    // Special
    EOF_TOKEN,
    ERROR
};

class Token {
public:
    TokenType type;
    std::string value;
    std::string filename;
    size_t line;
    size_t column;

    Token(TokenType type,
        const std::string& value,
        const std::string& filename,
        size_t line,
        size_t column)
        : type(type),
        value(value),
        filename(filename),
        line(line),
        column(column) {}

    // Default constructor for when we need to create empty tokens
    Token()
        : type(TokenType::ERROR),
        value(""),
        filename(""),
        line(0),
        column(0) {}

    std::string toString() const;

    // Helper functions for token type checking
    bool isKeyword() const {
        return type >= TokenType::DEF && type <= TokenType::NIL;
    }

    bool isType() const {
        return type >= TokenType::TYPE && type <= TokenType::RESULT;
    }

    bool isLiteral() const {
        return type >= TokenType::IDENTIFIER && type <= TokenType::BOOL_LITERAL;
    }

    bool isOperator() const {
        return type >= TokenType::PLUS && type <= TokenType::HASH;
    }

    bool isDelimiter() const {
        return type >= TokenType::LEFT_PAREN && type <= TokenType::RIGHT_BRACE;
    }
};

#endif // TOKEN_H
