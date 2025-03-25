// src/lexer/token.h
#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <unordered_map>

enum class TokenType {
	DEF, CLASS, IF, ELIF, ELSE, FOR, IN, WHILE, RETURN, IMPORT, FROM, MATCH, CASE,
	DEFAULT, ASYNC, AWAIT, CONST, LET, UNSAFE, INTERFACE, OVERRIDE, SPAWN, PURE,
	TRUE, FALSE, NIL, PRINT, TYPE, INT, INT8, INT16, INT32, INT64, UINT, UINT8,
	UINT16, UINT32, UINT64, FLOAT32, FLOAT64, BOOL, CHAR, STRING, LIST, MAP, SET,
	TUPLE, OPTION, RESULT, IDENTIFIER, INTEGER_LITERAL, FLOAT_LITERAL, STRING_LITERAL,
	BOOL_LITERAL, PLUS, MINUS, STAR, SLASH, PERCENT, EQUAL, PLUS_EQUAL, MINUS_EQUAL,
	STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL, BANG, BANG_EQUAL, EQUAL_EQUAL, GREATER,
	GREATER_EQUAL, LESS, LESS_EQUAL, AND, OR, ARROW, COLON, DOUBLE_COLON, DOT, COMMA,
	QUESTION, AT, HASH, SEMI_COLON, LEFT_PAREN, RIGHT_PAREN, LEFT_BRACKET, RIGHT_BRACKET,
	LEFT_BRACE, RIGHT_BRACE, INDENT, DEDENT, NEWLINE, EOF_TOKEN, ERROR
};

class Token {
public:
	Token() : type(TokenType::ERROR), value(""), filename(""), line(0), column(0) {}
	Token(TokenType t, std::string v, std::string f, int l, int c)
		: type(t), value(std::move(v)), filename(std::move(f)), line(l), column(c) {}

	TokenType type;
	std::string value;
	std::string filename;
	int line;
	int column;
};

inline const std::unordered_map<TokenType, std::string> tokenMap = {
 {TokenType::DEF, "DEF"}, {TokenType::CLASS, "CLASS"}, {TokenType::IF, "IF"},
 {TokenType::ELIF, "ELIF"}, {TokenType::ELSE, "ELSE"}, {TokenType::FOR, "FOR"},
 {TokenType::IN, "IN"}, {TokenType::WHILE, "WHILE"}, {TokenType::RETURN, "RETURN"},
 {TokenType::IMPORT, "IMPORT"}, {TokenType::FROM, "FROM"}, {TokenType::MATCH, "MATCH"},
 {TokenType::CASE, "CASE"}, {TokenType::DEFAULT, "DEFAULT"}, {TokenType::ASYNC, "ASYNC"},
 {TokenType::AWAIT, "AWAIT"}, {TokenType::CONST, "CONST"}, {TokenType::LET, "LET"},
 {TokenType::UNSAFE, "UNSAFE"}, {TokenType::INTERFACE, "INTERFACE"},
 {TokenType::OVERRIDE, "OVERRIDE"}, {TokenType::SPAWN, "SPAWN"}, {TokenType::PURE, "PURE"},
 {TokenType::TRUE, "TRUE"}, {TokenType::FALSE, "FALSE"}, {TokenType::NIL, "NIL"},
 {TokenType::PRINT, "PRINT"}, {TokenType::TYPE, "TYPE"}, {TokenType::INT, "INT"},
 {TokenType::INT8, "INT8"}, {TokenType::INT16, "INT16"}, {TokenType::INT32, "INT32"},
 {TokenType::INT64, "INT64"}, {TokenType::UINT, "UINT"}, {TokenType::UINT8, "UINT8"},
 {TokenType::UINT16, "UINT16"}, {TokenType::UINT32, "UINT32"}, {TokenType::UINT64, "UINT64"},
 {TokenType::FLOAT32, "FLOAT32"}, {TokenType::FLOAT64, "FLOAT64"}, {TokenType::BOOL, "BOOL"},
 {TokenType::CHAR, "CHAR"}, {TokenType::STRING, "STRING"}, {TokenType::LIST, "LIST"},
 {TokenType::MAP, "MAP"}, {TokenType::SET, "SET"}, {TokenType::TUPLE, "TUPLE"},
 {TokenType::OPTION, "OPTION"}, {TokenType::RESULT, "RESULT"},
 {TokenType::IDENTIFIER, "IDENTIFIER"}, {TokenType::INTEGER_LITERAL, "INTEGER_LITERAL"},
 {TokenType::FLOAT_LITERAL, "FLOAT_LITERAL"}, {TokenType::STRING_LITERAL, "STRING_LITERAL"},
 {TokenType::BOOL_LITERAL, "BOOL_LITERAL"}, {TokenType::PLUS, "+"}, {TokenType::MINUS, "-"},
 {TokenType::STAR, "*"}, {TokenType::SLASH, "/"}, {TokenType::PERCENT, "%"},
 {TokenType::EQUAL, "="}, {TokenType::PLUS_EQUAL, "+="}, {TokenType::MINUS_EQUAL, "-="},
 {TokenType::STAR_EQUAL, "*="}, {TokenType::SLASH_EQUAL, "/="}, {TokenType::PERCENT_EQUAL, "%="},
 {TokenType::BANG, "!"}, {TokenType::BANG_EQUAL, "!="}, {TokenType::EQUAL_EQUAL, "=="},
 {TokenType::GREATER, " >"}, {TokenType::GREATER_EQUAL, ">="}, {TokenType::LESS, "<"},
 {TokenType::LESS_EQUAL, "<="}, {TokenType::AND, "AND"}, {TokenType::OR, "OR"},
 {TokenType::ARROW, "->"}, {TokenType::COLON, ":"}, {TokenType::DOUBLE_COLON, ":: "},
 {TokenType::DOT, "."}, {TokenType::COMMA, ","}, {TokenType::QUESTION, "?"},
 {TokenType::AT, "@"}, {TokenType::HASH, "#"}, {TokenType::SEMI_COLON, ";"},
 {TokenType::LEFT_PAREN, "("}, {TokenType::RIGHT_PAREN, ")"}, {TokenType::LEFT_BRACKET, "["},
 {TokenType::RIGHT_BRACKET, "]"}, {TokenType::LEFT_BRACE, "{"}, {TokenType::RIGHT_BRACE, "}"},
 {TokenType::INDENT, "INDENT"}, {TokenType::DEDENT, "DEDENT"}, {TokenType::NEWLINE, "NEWLINE"},
 {TokenType::EOF_TOKEN, "EOF"}, {TokenType::ERROR, "ERROR"}
};

#endif // TOKEN_H