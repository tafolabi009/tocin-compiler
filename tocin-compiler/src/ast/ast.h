// tocin-compiler/src/ast/ast.h
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include "../lexer/token.h"

namespace ast {

    // Forward declarations
    class Expression;
    class Statement;
    class Type;

    using ExprPtr = std::shared_ptr<Expression>;
    using StmtPtr = std::shared_ptr<Statement>;
    using TypePtr = std::shared_ptr<Type>;
    using ExprList = std::vector<ExprPtr>;
    using StmtList = std::vector<StmtPtr>;

    // AST Visitor interface
    class Visitor {
    public:
        virtual ~Visitor() = default;

        // Expressions
        virtual void visitBinaryExpr(class BinaryExpr* expr) = 0;
        virtual void visitGroupingExpr(class GroupingExpr* expr) = 0;
        virtual void visitLiteralExpr(class LiteralExpr* expr) = 0;
        virtual void visitUnaryExpr(class UnaryExpr* expr) = 0;
        virtual void visitVariableExpr(class VariableExpr* expr) = 0;
        virtual void visitAssignExpr(class AssignExpr* expr) = 0;
        virtual void visitCallExpr(class CallExpr* expr) = 0;
        virtual void visitGetExpr(class GetExpr* expr) = 0;
        virtual void visitSetExpr(class SetExpr* expr) = 0;
        virtual void visitListExpr(class ListExpr* expr) = 0;
        virtual void visitDictionaryExpr(class DictionaryExpr* expr) = 0;
        virtual void visitLambdaExpr(class LambdaExpr* expr) = 0;

        // Statements
        virtual void visitExpressionStmt(class ExpressionStmt* stmt) = 0;
        virtual void visitVariableStmt(class VariableStmt* stmt) = 0;
        virtual void visitBlockStmt(class BlockStmt* stmt) = 0;
        virtual void visitIfStmt(class IfStmt* stmt) = 0;
        virtual void visitWhileStmt(class WhileStmt* stmt) = 0;
        virtual void visitForStmt(class ForStmt* stmt) = 0;
        virtual void visitFunctionStmt(class FunctionStmt* stmt) = 0;
        virtual void visitReturnStmt(class ReturnStmt* stmt) = 0;
        virtual void visitClassStmt(class ClassStmt* stmt) = 0;
        virtual void visitImportStmt(class ImportStmt* stmt) = 0;
        virtual void visitMatchStmt(class MatchStmt* stmt) = 0;
    };

    // Base class for all AST nodes
    class Node {
    public:
        Token token;

        Node(const Token& token) : token(token) {}
        virtual ~Node() = default;
    };

    // Base class for all expressions
    class Expression : public Node {
    public:
        Expression(const Token& token) : Node(token) {}
        virtual void accept(Visitor& visitor) = 0;
    };

    // Base class for all statements
    class Statement : public Node {
    public:
        Statement(const Token& token) : Node(token) {}
        virtual void accept(Visitor& visitor) = 0;
    };

    // Base class for all types
    class Type : public Node {
    public:
        Type(const Token& token) : Node(token) {}
        virtual std::string toString() const = 0;
    };

    // Simple type (e.g., int, string)
    class SimpleType : public Type {
    public:
        SimpleType(const Token& token) : Type(token) {}

        std::string toString() const override {
            return token.value;
        }
    };

    // Generic type (e.g., list[int], map[string, int])
    class GenericType : public Type {
    public:
        std::string name;
        std::vector<TypePtr> typeArguments;

        GenericType(const Token& token, std::string name, std::vector<TypePtr> typeArguments)
            : Type(token), name(std::move(name)), typeArguments(std::move(typeArguments)) {}

        std::string toString() const override {
            std::string result = name + "[";
            for (size_t i = 0; i < typeArguments.size(); i++) {
                if (i > 0) result += ", ";
                result += typeArguments[i]->toString();
            }
            result += "]";
            return result;
        }
    };

    // Parameter in a function definition
    struct Parameter {
        std::string name;
        TypePtr type;

        Parameter(std::string name, TypePtr type)
            : name(std::move(name)), type(std::move(type)) {}
    };

    // Expression AST nodes
    class BinaryExpr : public Expression {
    public:
        ExprPtr left;
        Token op;
        ExprPtr right;

        BinaryExpr(ExprPtr left, Token op, ExprPtr right)
            : Expression(op), left(std::move(left)), op(op), right(std::move(right)) {}

        void accept(Visitor& visitor) override {
            visitor.visitBinaryExpr(this);
        }
    };

    class GroupingExpr : public Expression {
    public:
        ExprPtr expression;

        GroupingExpr(Token token, ExprPtr expression)
            : Expression(token), expression(std::move(expression)) {}

        void accept(Visitor& visitor) override {
            visitor.visitGroupingExpr(this);
        }
    };

    class LiteralExpr : public Expression {
    public:
        enum class LiteralType {
            INTEGER, FLOAT, STRING, BOOLEAN, NIL
        };

        LiteralType literalType;
        std::string value;

        LiteralExpr(Token token, LiteralType literalType, std::string value)
            : Expression(token), literalType(literalType), value(std::move(value)) {}

        void accept(Visitor& visitor) override {
            visitor.visitLiteralExpr(this);
        }
    };

    class UnaryExpr : public Expression {
    public:
        Token op;
        ExprPtr right;

        UnaryExpr(Token op, ExprPtr right)
            : Expression(op), op(op), right(std::move(right)) {}

        void accept(Visitor& visitor) override {
            visitor.visitUnaryExpr(this);
        }
    };

    class VariableExpr : public Expression {
    public:
        std::string name;

        VariableExpr(Token token)
            : Expression(token), name(token.value) {}

        void accept(Visitor& visitor) override {
            visitor.visitVariableExpr(this);
        }
    };

    class AssignExpr : public Expression {
    public:
        std::string name;
        ExprPtr value;

        AssignExpr(Token token, ExprPtr value)
            : Expression(token), name(token.value), value(std::move(value)) {}

        void accept(Visitor& visitor) override {
            visitor.visitAssignExpr(this);
        }
    };

    class CallExpr : public Expression {
    public:
        ExprPtr callee;
        std::vector<ExprPtr> arguments;

        CallExpr(Token token, ExprPtr callee, std::vector<ExprPtr> arguments)
            : Expression(token), callee(std::move(callee)), arguments(std::move(arguments)) {}

        void accept(Visitor& visitor) override {
            visitor.visitCallExpr(this);
        }
    };

    class GetExpr : public Expression {
    public:
        ExprPtr object;
        std::string name;

        GetExpr(Token token, ExprPtr object)
            : Expression(token), object(std::move(object)), name(token.value) {}

        void accept(Visitor& visitor) override {
            visitor.visitGetExpr(this);
        }
    };

    class SetExpr : public Expression {
    public:
        ExprPtr object;
        std::string name;
        ExprPtr value;

        SetExpr(Token token, ExprPtr object, ExprPtr value)
            : Expression(token), object(std::move(object)), name(token.value), value(std::move(value)) {}

        void accept(Visitor& visitor) override {
            visitor.visitSetExpr(this);
        }
    };

    class ListExpr : public Expression {
    public:
        std::vector<ExprPtr> elements;

        ListExpr(Token token, std::vector<ExprPtr> elements)
            : Expression(token), elements(std::move(elements)) {}

        void accept(Visitor& visitor) override {
            visitor.visitListExpr(this);
        }
    };

    class DictionaryExpr : public Expression {
    public:
        std::vector<std::pair<ExprPtr, ExprPtr>> entries;

        DictionaryExpr(Token token, std::vector<std::pair<ExprPtr, ExprPtr>> entries)
            : Expression(token), entries(std::move(entries)) {}

        void accept(Visitor& visitor) override {
            visitor.visitDictionaryExpr(this);
        }
    };

    class LambdaExpr : public Expression {
    public:
        std::vector<Parameter> parameters;
        TypePtr returnType;
        StmtPtr body;

        LambdaExpr(Token token, std::vector<Parameter> parameters, TypePtr returnType, StmtPtr body)
            : Expression(token), parameters(std::move(parameters)), returnType(std::move(returnType)), body(std::move(body)) {}

        void accept(Visitor& visitor) override {
            visitor.visitLambdaExpr(this);
        }
    };

    // Statement AST nodes
    class ExpressionStmt : public Statement {
    public:
        ExprPtr expression;

        ExpressionStmt(Token token, ExprPtr expression)
            : Statement(token), expression(std::move(expression)) {}

        void accept(Visitor& visitor) override {
            visitor.visitExpressionStmt(this);
        }
    };

    class VariableStmt : public Statement {
    public:
        std::string name;
        TypePtr type;
        ExprPtr initializer;
        bool isConstant;

        VariableStmt(Token token, TypePtr type, ExprPtr initializer, bool isConstant)
            : Statement(token), name(token.value), type(std::move(type)),
            initializer(std::move(initializer)), isConstant(isConstant) {}

        void accept(Visitor& visitor) override {
            visitor.visitVariableStmt(this);
        }
    };

    class BlockStmt : public Statement {
    public:
        std::vector<StmtPtr> statements;

        BlockStmt(Token token, std::vector<StmtPtr> statements)
            : Statement(token), statements(std::move(statements)) {}

        void accept(Visitor& visitor) override {
            visitor.visitBlockStmt(this);
        }
    };

    class IfStmt : public Statement {
    public:
        ExprPtr condition;
        StmtPtr thenBranch;
        StmtPtr elseBranch;
        std::vector<std::pair<ExprPtr, StmtPtr>> elifBranches;

        IfStmt(Token token, ExprPtr condition, StmtPtr thenBranch,
            std::vector<std::pair<ExprPtr, StmtPtr>> elifBranches, StmtPtr elseBranch)
            : Statement(token), condition(std::move(condition)), thenBranch(std::move(thenBranch)),
            elseBranch(std::move(elseBranch)), elifBranches(std::move(elifBranches)) {}

        void accept(Visitor& visitor) override {
            visitor.visitIfStmt(this);
        }
    };

    class WhileStmt : public Statement {
    public:
        ExprPtr condition;
        StmtPtr body;

        WhileStmt(Token token, ExprPtr condition, StmtPtr body)
            : Statement(token), condition(std::move(condition)), body(std::move(body)) {}

        void accept(Visitor& visitor) override {
            visitor.visitWhileStmt(this);
        }
    };

    class ForStmt : public Statement {
    public:
        std::string variable;
        TypePtr variableType;
        ExprPtr iterable;
        StmtPtr body;

        ForStmt(Token token, std::string variable, TypePtr variableType,
            ExprPtr iterable, StmtPtr body)
            : Statement(token), variable(std::move(variable)), variableType(std::move(variableType)),
            iterable(std::move(iterable)), body(std::move(body)) {}

        void accept(Visitor& visitor) override {
            visitor.visitForStmt(this);
        }
    };

    class FunctionStmt : public Statement {
    public:
        std::string name;
        std::vector<Parameter> parameters;
        TypePtr returnType;
        StmtPtr body;
        bool isAsync;
        bool isPure;

        FunctionStmt(Token token, std::vector<Parameter> parameters, TypePtr returnType,
            StmtPtr body, bool isAsync = false, bool isPure = false)
            : Statement(token), name(token.value), parameters(std::move(parameters)),
            returnType(std::move(returnType)), body(std::move(body)),
            isAsync(isAsync), isPure(isPure) {}

        void accept(Visitor& visitor) override {
            visitor.visitFunctionStmt(this);
        }
    };

    class ReturnStmt : public Statement {
    public:
        ExprPtr value;

        ReturnStmt(Token token, ExprPtr value)
            : Statement(token), value(std::move(value)) {}

        void accept(Visitor& visitor) override {
            visitor.visitReturnStmt(this);
        }
    };

    class ClassStmt : public Statement {
    public:
        std::string name;
        std::vector<TypePtr> superclasses;
        std::vector<std::shared_ptr<VariableStmt>> fields;
        std::vector<std::shared_ptr<FunctionStmt>> methods;

        ClassStmt(Token token, std::vector<TypePtr> superclasses,
            std::vector<std::shared_ptr<VariableStmt>> fields,
            std::vector<std::shared_ptr<FunctionStmt>> methods)
            : Statement(token), name(token.value), superclasses(std::move(superclasses)),
            fields(std::move(fields)), methods(std::move(methods)) {}

        void accept(Visitor& visitor) override {
            visitor.visitClassStmt(this);
        }
    };

    class ImportStmt : public Statement {
    public:
        std::string module;
        std::vector<std::pair<std::string, std::string>> imports; // original name, alias

        ImportStmt(Token token, std::string module,
            std::vector<std::pair<std::string, std::string>> imports)
            : Statement(token), module(std::move(module)), imports(std::move(imports)) {}

        void accept(Visitor& visitor) override {
            visitor.visitImportStmt(this);
        }
    };

    class MatchCase {
    public:
        ExprPtr pattern;
        StmtPtr body;

        MatchCase(ExprPtr pattern, StmtPtr body)
            : pattern(std::move(pattern)), body(std::move(body)) {}
    };

    class MatchStmt : public Statement {
    public:
        ExprPtr value;
        std::vector<MatchCase> cases;
        StmtPtr defaultCase;

        MatchStmt(Token token, ExprPtr value, std::vector<MatchCase> cases, StmtPtr defaultCase)
            : Statement(token), value(std::move(value)), cases(std::move(cases)),
            defaultCase(std::move(defaultCase)) {}

        void accept(Visitor& visitor) override {
            visitor.visitMatchStmt(this);
        }
    };

} // namespace ast