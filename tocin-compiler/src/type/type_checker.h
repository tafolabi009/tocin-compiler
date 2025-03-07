// tocin-compiler/src/type/type_checker.h
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
#include "../ast/ast.h"

class TypeCheckError : public std::runtime_error {
public:
    TypeCheckError(const std::string& message) : std::runtime_error(message) {}
};

class TypeChecker : public ast::Visitor {
public:
    TypeChecker();
    void check(std::shared_ptr<ast::Statement> ast);

    // New: Run additional static analysis checks.
    void analyze(std::shared_ptr<ast::Statement> ast);

private:
    // Environment for type checking
    class Environment {
    public:
        Environment() : enclosing(nullptr) {}
        Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

        void define(const std::string& name, std::shared_ptr<ast::Type> type, bool isConstant = false);
        std::shared_ptr<ast::Type> get(const std::string& name);
        bool isConstant(const std::string& name);

    private:
        std::unordered_map<std::string, std::shared_ptr<ast::Type>> values;
        std::unordered_map<std::string, bool> constants;
        std::shared_ptr<Environment> enclosing;
    };

    std::shared_ptr<Environment> currentEnvironment;
    std::shared_ptr<ast::Type> currentFunctionReturnType;
    bool insideLoop = false;
    std::stack<std::shared_ptr<ast::Type>> expressionTypes;

    // Helper methods
    void beginScope();
    void endScope();
    std::shared_ptr<ast::Type> checkExpression(std::shared_ptr<ast::Expression> expr);
    bool isTypeAssignable(std::shared_ptr<ast::Type> target, std::shared_ptr<ast::Type> source);
    TypeCheckError error(const ast::Node& node, const std::string& message);

    // Visitor implementation for expressions
    void visitBinaryExpr(ast::BinaryExpr* expr) override;
    void visitGroupingExpr(ast::GroupingExpr* expr) override;
    void visitLiteralExpr(ast::LiteralExpr* expr) override;
    void visitUnaryExpr(ast::UnaryExpr* expr) override;
    void visitVariableExpr(ast::VariableExpr* expr) override;
    void visitAssignExpr(ast::AssignExpr* expr) override;
    void visitCallExpr(ast::CallExpr* expr) override;
    void visitGetExpr(ast::GetExpr* expr) override;
    void visitSetExpr(ast::SetExpr* expr) override;
    void visitListExpr(ast::ListExpr* expr) override;
    void visitDictionaryExpr(ast::DictionaryExpr* expr) override;
    void visitLambdaExpr(ast::LambdaExpr* expr) override;

    // Visitor implementation for statements
    void visitExpressionStmt(ast::ExpressionStmt* stmt) override;
    void visitVariableStmt(ast::VariableStmt* stmt) override;
    void visitBlockStmt(ast::BlockStmt* stmt) override;
    void visitIfStmt(ast::IfStmt* stmt) override;
    void visitWhileStmt(ast::WhileStmt* stmt) override;
    void visitForStmt(ast::ForStmt* stmt) override;
    void visitFunctionStmt(ast::FunctionStmt* stmt) override;
    void visitReturnStmt(ast::ReturnStmt* stmt) override;
    void visitClassStmt(ast::ClassStmt* stmt) override;
    void visitImportStmt(ast::ImportStmt* stmt) override;
    void visitMatchStmt(ast::MatchStmt* stmt) override;
};