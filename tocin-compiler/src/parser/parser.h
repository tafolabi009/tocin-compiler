// tocin-compiler/src/parser/parser.h
#pragma once

#include <vector>
#include <memory>
#include <stdexcept>
#include "../lexer/token.h"
#include "../ast/ast.h"

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::shared_ptr<ast::Statement> parse();

private:
    std::vector<Token> tokens;
    size_t current = 0;
    int currentIndentLevel = 0;

    // Utility methods
    Token peek() const;
    Token previous() const;
    Token advance();
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string& message);
    ParseError error(const Token& token, const std::string& message);
    void synchronize();

    // Parsing methods
    std::shared_ptr<ast::Statement> declaration();
    std::shared_ptr<ast::VariableStmt> variableDeclaration();
    std::shared_ptr<ast::FunctionStmt> functionDeclaration();
    std::shared_ptr<ast::ClassStmt> classDeclaration();
    std::shared_ptr<ast::Statement> statement();
    std::shared_ptr<ast::ExpressionStmt> expressionStatement();
    std::shared_ptr<ast::BlockStmt> block();
    std::shared_ptr<ast::IfStmt> ifStatement();
    std::shared_ptr<ast::WhileStmt> whileStatement();
    std::shared_ptr<ast::ForStmt> forStatement();
    std::shared_ptr<ast::ReturnStmt> returnStatement();
    std::shared_ptr<ast::ImportStmt> importStatement();
    std::shared_ptr<ast::MatchStmt> matchStatement();

    // Expression parsing
    std::shared_ptr<ast::Expression> expression();
    std::shared_ptr<ast::Expression> assignment();
    std::shared_ptr<ast::Expression> logicalOr();
    std::shared_ptr<ast::Expression> logicalAnd();
    std::shared_ptr<ast::Expression> equality();
    std::shared_ptr<ast::Expression> comparison();
    std::shared_ptr<ast::Expression> addition();
    std::shared_ptr<ast::Expression> multiplication();
    std::shared_ptr<ast::Expression> unary();
    std::shared_ptr<ast::Expression> call();
    std::shared_ptr<ast::Expression> primary();
    std::shared_ptr<ast::Expression> list();
    std::shared_ptr<ast::Expression> dictionary();
    std::shared_ptr<ast::Expression> lambda();

    // Type parsing
    std::shared_ptr<ast::Type> parseType();
    std::shared_ptr<ast::Type> parseSimpleType();
    std::shared_ptr<ast::Type> parseGenericType(const Token& nameToken);
};