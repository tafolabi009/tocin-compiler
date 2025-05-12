#include "token.h"
#include <sstream>
#include <iomanip>

namespace lexer {

    std::string Token::toString() const {
        std::stringstream ss;
        std::string typeStr;
        switch (type) {
        case TokenType::LEFT_PAREN: typeStr = "LEFT_PAREN"; break;
        case TokenType::RIGHT_PAREN: typeStr = "RIGHT_PAREN"; break;
        case TokenType::LEFT_BRACE: typeStr = "LEFT_BRACE"; break;
        case TokenType::RIGHT_BRACE: typeStr = "RIGHT_BRACE"; break;
        case TokenType::LEFT_BRACKET: typeStr = "LEFT_BRACKET"; break;
        case TokenType::RIGHT_BRACKET: typeStr = "RIGHT_BRACKET"; break;
        case TokenType::COMMA: typeStr = "COMMA"; break;
        case TokenType::DOT: typeStr = "DOT"; break;
        case TokenType::SEMI_COLON: typeStr = "SEMI_COLON"; break;
        case TokenType::COLON: typeStr = "COLON"; break;
        case TokenType::PLUS: typeStr = "PLUS"; break;
        case TokenType::PLUS_EQUAL: typeStr = "PLUS_EQUAL"; break;
        case TokenType::MINUS: typeStr = "MINUS"; break;
        case TokenType::MINUS_EQUAL: typeStr = "MINUS_EQUAL"; break;
        case TokenType::STAR: typeStr = "STAR"; break;
        case TokenType::STAR_EQUAL: typeStr = "STAR_EQUAL"; break;
        case TokenType::SLASH: typeStr = "SLASH"; break;
        case TokenType::SLASH_EQUAL: typeStr = "SLASH_EQUAL"; break;
        case TokenType::PERCENT: typeStr = "PERCENT"; break;
        case TokenType::PERCENT_EQUAL: typeStr = "PERCENT_EQUAL"; break;
        case TokenType::EQUAL: typeStr = "EQUAL"; break;
        case TokenType::EQUAL_EQUAL: typeStr = "EQUAL_EQUAL"; break;
        case TokenType::BANG: typeStr = "BANG"; break;
        case TokenType::BANG_EQUAL: typeStr = "BANG_EQUAL"; break;
        case TokenType::LESS: typeStr = "LESS"; break;
        case TokenType::LESS_EQUAL: typeStr = "LESS_EQUAL"; break;
        case TokenType::GREATER: typeStr = "GREATER"; break;
        case TokenType::GREATER_EQUAL: typeStr = "GREATER_EQUAL"; break;
        case TokenType::DOUBLE_COLON: typeStr = "DOUBLE_COLON"; break;
        case TokenType::ARROW: typeStr = "ARROW"; break;
        case TokenType::AND: typeStr = "AND"; break;
        case TokenType::OR: typeStr = "OR"; break;
        case TokenType::IDENTIFIER: typeStr = "IDENTIFIER"; break;
        case TokenType::STRING: typeStr = "STRING"; break;
        case TokenType::INT: typeStr = "INT"; break;
        case TokenType::FLOAT64: typeStr = "FLOAT64"; break;
        case TokenType::FLOAT32: typeStr = "FLOAT32"; break;
        case TokenType::LET: typeStr = "LET"; break;
        case TokenType::DEF: typeStr = "DEF"; break;
        case TokenType::ASYNC: typeStr = "ASYNC"; break;
        case TokenType::AWAIT: typeStr = "AWAIT"; break;
        case TokenType::CLASS: typeStr = "CLASS"; break;
        case TokenType::IF: typeStr = "IF"; break;
        case TokenType::ELIF: typeStr = "ELIF"; break;
        case TokenType::ELSE: typeStr = "ELSE"; break;
        case TokenType::WHILE: typeStr = "WHILE"; break;
        case TokenType::FOR: typeStr = "FOR"; break;
        case TokenType::IN: typeStr = "IN"; break;
        case TokenType::RETURN: typeStr = "RETURN"; break;
        case TokenType::IMPORT: typeStr = "IMPORT"; break;
        case TokenType::FROM: typeStr = "FROM"; break;
        case TokenType::MATCH: typeStr = "MATCH"; break;
        case TokenType::CASE: typeStr = "CASE"; break;
        case TokenType::DEFAULT: typeStr = "DEFAULT"; break;
        case TokenType::CONST: typeStr = "CONST"; break;
        case TokenType::TRUE: typeStr = "TRUE"; break;
        case TokenType::FALSE: typeStr = "FALSE"; break;
        case TokenType::NIL: typeStr = "NIL"; break;
        case TokenType::LAMBDA: typeStr = "LAMBDA"; break;
        case TokenType::PRINT: typeStr = "PRINT"; break;
        case TokenType::INDENT: typeStr = "INDENT"; break;
        case TokenType::DEDENT: typeStr = "DEDENT"; break;
        case TokenType::ERROR: typeStr = "ERROR"; break;
        case TokenType::EOF_TOKEN: typeStr = "EOF_TOKEN"; break;
        }

        // Escape special characters in value
        std::string escapedValue;
        for (char c : value) {
            switch (c) {
            case '\n': escapedValue += "\\n"; break;
            case '\t': escapedValue += "\\t"; break;
            case '\r': escapedValue += "\\r"; break;
            case '\\': escapedValue += "\\\\"; break;
            case '"': escapedValue += "\\\""; break;
            default: escapedValue += c; break;
            }
        }

        ss << "[" << typeStr << " \"" << escapedValue << "\" at " << filename << ":"
            << line << ":" << column << "]";
        return ss.str();
    }

} // namespace lexer