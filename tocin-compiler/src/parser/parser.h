// File: src/parser/parser.h
#pragma once

#include <vector>
#include <memory>
#include <initializer_list>
#include "../lexer/token.h"
#include "../ast/ast.h"
#include "../compiler/compilation_context.h"

namespace parser {

    /// @brief Parses a sequence of tokens into an Abstract Syntax Tree (AST)
    class Parser {
    public:
        /// @brief Constructs a parser with a token list and compilation context
        /// @param tokens The list of tokens to parse
        /// @param context Compilation context for error reporting and configuration
        explicit Parser(const std::vector<Token>& tokens, CompilationContext& context);

        /// @brief Parses the tokens into an AST statement
        /// @return A shared pointer to the parsed statement, or nullptr on error
        std::shared_ptr<ast::Statement> parse();

    private:
        const std::vector<Token> tokens;  ///< Immutable list of input tokens
        size_t current;                   ///< Current position in the token list
        CompilationContext& context;      ///< Reference to compilation context

        // Declaration parsing
        std::shared_ptr<ast::Statement> declaration();
        std::shared_ptr<ast::VariableStmt> variableDeclaration();
        std::shared_ptr<ast::FunctionStmt> functionDeclaration();
        std::shared_ptr<ast::ClassStmt> classDeclaration();

        // Statement parsing
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
        std::shared_ptr<ast::Expression> finishCall(std::shared_ptr<ast::Expression> callee);
        std::shared_ptr<ast::Expression> primary();
        std::shared_ptr<ast::Expression> list();
        std::shared_ptr<ast::Expression> dictionary();
        std::shared_ptr<ast::Expression> lambda();

        // Type parsing
        std::shared_ptr<ast::Type> parseType();
        std::shared_ptr<ast::Type> parseGenericType(const Token& nameToken);

        // Token handling and error reporting
        bool check(TokenType type) const;
        Token peek() const;
        Token previous() const;
        Token advance();
        bool isAtEnd() const;
        bool match(TokenType type);
        bool match(std::initializer_list<TokenType> types);
        Token consume(TokenType type, const std::string& message);
        void reportError(const Token& token, const std::string& message, error::ErrorSeverity severity);
        void synchronize();
    };

} // namespace parser