#include "token.h"

std::string Token::toString() const {
    // Use the tokenTypeToString map for readable type names
    auto typeIt = tokenTypeToString.find(type);
    std::string typeStr = (typeIt != tokenTypeToString.end()) ? typeIt->second : "UNKNOWN";

    // Efficient string concatenation
    std::string result = "[" + typeStr + " '" + value + "' at " +
        filename + ":" + std::to_string(line) + ":" +
        std::to_string(column) + "]";
    return result;
}