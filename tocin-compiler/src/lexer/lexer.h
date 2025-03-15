#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "token.h"  // Token definitions are provided here

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
