// tocin-compiler/src/lexer/token.cpp
#include "token.h"
#include <sstream>

std::string Token::toString() const {
    std::stringstream ss;
    ss << "[" << static_cast<int>(type) << " '" << value << "' at "
        << filename << ":" << line << ":" << column << "]";
    return ss.str();
}