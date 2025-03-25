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

    /// @brief Interface for visiting AST nodes
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

    /// @brief Base class for all AST nodes
    class Node {
    public:
        Token token;

        explicit Node(const Token& token) : token(token) {}
        virtual ~Node() = default;
    };

    /// @brief Base class for expression nodes
    class Expression : public Node {
    public:
        explicit Expression(const Token& token) : Node(token) {}
        virtual void accept(Visitor& visitor) = 0;
    };

    /// @brief Base class for statement nodes
    class Statement : public Node {
    public:
        explicit Statement(const Token& token) : Node(token) {}
        virtual void accept(Visitor& visitor) = 0;
    };

    /// @brief Base class for type nodes
    class Type : public Node {
    public:
        explicit Type(const Token& token) : Node(token) {}
        virtual std::string toString() const = 0;
    };

    /// @brief Represents a simple type (e.g., int, string)
    class SimpleType : public Type {
    public:
        explicit SimpleType(const Token& token) : Type(token) {}
        std::string toString() const override { return token.value; }
    };

    /// @brief Represents a generic type (e.g., list[int])
    class GenericType : public Type {
    public:
        std::string name;
        std::vector<TypePtr> typeArguments;

        GenericType(const Token& token, std::string name, std::vector<TypePtr> typeArguments)
            : Type(token), name(std::move(name)), typeArguments(std::move(typeArguments)) {
            if (typeArguments.empty()) {
                throw std::runtime_error("GenericType must have at least one type argument");
            }
        }

        std::string toString() const override {
            std::string result = name + "[";
            for (size_t i = 0; i < typeArguments.size(); ++i) {
                if (i > 0) result += ", ";
                result += typeArguments[i]->toString();
            }
            return result + "]";
        }
    };

    /// @brief Represents a function parameter
    struct Parameter {
        std::string name;
        TypePtr type;

        Parameter(std::string name, TypePtr type)
            : name(std::move(name)), type(std::move(type)) {
            if (!type) throw std::runtime_error("Parameter type cannot be null");
        }
    };

    // Expression AST nodes
    /// @brief Binary operation (e.g., a + b)
    class BinaryExpr : public Expression {
    public:
        ExprPtr left;
        Token op;
        ExprPtr right;

        BinaryExpr(ExprPtr left, Token op, ExprPtr right)
            : Expression(op), left(std::move(left)), op(op), right(std::move(right)) {
            if (!left || !right) throw std::runtime_error("BinaryExpr operands cannot be null");
        }

        void accept(Visitor& visitor) override { visitor.visitBinaryExpr(this); }
    };

    /// @brief Parenthesized expression (e.g., (expr))
    class GroupingExpr : public Expression {
    public:
        ExprPtr expression;

        GroupingExpr(Token token, ExprPtr expression)
            : Expression(token), expression(std::move(expression)) {
            if (!expression) throw std::runtime_error("GroupingExpr expression cannot be null");
        }

        void accept(Visitor& visitor) override { visitor.visitGroupingExpr(this); }
    };

    /// @brief Literal value (e.g., 42, "hello")
    class LiteralExpr : public Expression {
    public:
        enum class LiteralType { INTEGER, FLOAT, STRING, BOOLEAN, NIL };

        LiteralType literalType;
        std::string value;

        LiteralExpr(Token token, LiteralType literalType, std::string value)
            : Expression(token), literalType(literalType), value(std::move(value)) {}

        void accept(Visitor& visitor) override { visitor.visitLiteralExpr(this); }
    };

    /// @brief Unary operation (e.g., -x)
    class UnaryExpr : public Expression {
    public:
        Token op;
        ExprPtr right;

        UnaryExpr(Token op, ExprPtr right)
            : Expression(op), op(op), right(std::move(right)) {
            if (!right) throw std::runtime_error("UnaryExpr operand cannot be null");
        }

        void accept(Visitor& visitor) override { visitor.visitUnaryExpr(this); }
    };

    /// @brief Variable reference (e.g., x)
    class VariableExpr : public Expression {
    public:
        std::string name;

        explicit VariableExpr(const Token& token)
            : Expression(token), name(token.value) {
            if (name.empty()) throw std::runtime_error("VariableExpr name cannot be empty");
        }

        void accept(Visitor& visitor) override { visitor.visitVariableExpr(this); }
    };

    /// @brief Assignment (e.g., x = 5)
    class AssignExpr : public Expression {
    public:
        std::string name;
        ExprPtr value;

        AssignExpr(Token token, ExprPtr value)
            : Expression(token), name(token.value), value(std::move(value)) {
            if (!value) throw std::runtime_error("AssignExpr value cannot be null");
            if (name.empty()) throw std::runtime_error("AssignExpr name cannot be empty");
        }

        void accept(Visitor& visitor) override { visitor.visitAssignExpr(this); }
    };

    /// @brief Function call (e.g., foo(1, 2))
    class CallExpr : public Expression {
    public:
        ExprPtr callee;
        std::vector<ExprPtr> arguments;

        CallExpr(Token token, ExprPtr callee, std::vector<ExprPtr> arguments)
            : Expression(token), callee(std::move(callee)), arguments(std::move(arguments)) {
            if (!callee) throw std::runtime_error("CallExpr callee cannot be null");
        }

        void accept(Visitor& visitor) override { visitor.visitCallExpr(this); }
    };

    /// @brief Property access (e.g., obj.field)
    class GetExpr : public Expression {
    public:
        ExprPtr object;
        std::string name;

        GetExpr(Token token, ExprPtr object)
            : Expression(token), object(std::move(object)), name(token.value) {
            if (!object) throw std::runtime_error("GetExpr object cannot be null");
            if (name.empty()) throw std::runtime_error("GetExpr name cannot be empty");
        }

        void accept(Visitor& visitor) override { visitor.visitGetExpr(this); }
    };

    /// @brief Property assignment (e.g., obj.field = value)
    class SetExpr : public Expression {
    public:
        ExprPtr object;
        std::string name;
        ExprPtr value;

        SetExpr(Token token, ExprPtr object, ExprPtr value)
            : Expression(token), object(std::move(object)), name(token.value), value(std::move(value)) {
            if (!object || !value) throw std::runtime_error("SetExpr object or value cannot be null");
            if (name.empty()) throw std::runtime_error("SetExpr name cannot be empty");
        }

        void accept(Visitor& visitor) override { visitor.visitSetExpr(this); }
    };

    /// @brief List literal (e.g., [1, 2, 3])
    class ListExpr : public Expression {
    public:
        std::vector<ExprPtr> elements;

        explicit ListExpr(Token token, std::vector<ExprPtr> elements = {})
            : Expression(token), elements(std::move(elements)) {}

        void accept(Visitor& visitor) override { visitor.visitListExpr(this); }
    };

    /// @brief Dictionary literal (e.g., {"key": value})
    class DictionaryExpr : public Expression {
    public:
        std::vector<std::pair<ExprPtr, ExprPtr>> entries;

        explicit DictionaryExpr(Token token, std::vector<std::pair<ExprPtr, ExprPtr>> entries = {})
            : Expression(token), entries(std::move(entries)) {}

        void accept(Visitor& visitor) override { visitor.visitDictionaryExpr(this); }
    };

    /// @brief Lambda expression (e.g., lambda x: x + 1)
    class LambdaExpr : public Expression {
    public:
        std::vector<Parameter> parameters;
        TypePtr returnType;
        StmtPtr body;

        LambdaExpr(Token token, std::vector<Parameter> parameters, TypePtr returnType, StmtPtr body)
            : Expression(token), parameters(std::move(parameters)), returnType(std::move(returnType)),
            body(std::move(body)) {
            if (!body) throw std::runtime_error("LambdaExpr body cannot be null");
        }

        void accept(Visitor& visitor) override { visitor.visitLambdaExpr(this); }
    };

    // Statement AST nodes
    /// @brief Expression statement (e.g., foo())
    class ExpressionStmt : public Statement {
    public:
        ExprPtr expression;

        ExpressionStmt(Token token, ExprPtr expression)
            : Statement(token), expression(std::move(expression)) {
            if (!expression) throw std::runtime_error("ExpressionStmt expression cannot be null");
        }

        void accept(Visitor& visitor) override { visitor.visitExpressionStmt(this); }
    };

    /// @brief Variable declaration (e.g., let x: int = 5)
    class VariableStmt : public Statement {
    public:
        std::string name;
        TypePtr type;
        ExprPtr initializer;
        bool isConstant;

        VariableStmt(Token token, TypePtr type, ExprPtr initializer, bool isConstant)
            : Statement(token), name(token.value), type(std::move(type)),
            initializer(std::move(initializer)), isConstant(isConstant) {
            if (name.empty()) throw std::runtime_error("VariableStmt name cannot be empty");
        }

        void accept(Visitor& visitor) override { visitor.visitVariableStmt(this); }
    };

    /// @brief Block of statements (e.g., { stmt1; stmt2 })
    class BlockStmt : public Statement {
    public:
        std::vector<StmtPtr> statements;

        explicit BlockStmt(Token token, std::vector<StmtPtr> statements = {})
            : Statement(token), statements(std::move(statements)) {}

        void accept(Visitor& visitor) override { visitor.visitBlockStmt(this); }
    };

    /// @brief If statement (e.g., if cond { ... } else { ... })
    class IfStmt : public Statement {
    public:
        ExprPtr condition;
        StmtPtr thenBranch;
        StmtPtr elseBranch;
        std::vector<std::pair<ExprPtr, StmtPtr>> elifBranches;

        IfStmt(Token token, ExprPtr condition, StmtPtr thenBranch,
            std::vector<std::pair<ExprPtr, StmtPtr>> elifBranches = {}, StmtPtr elseBranch = nullptr)
            : Statement(token), condition(std::move(condition)), thenBranch(std::move(thenBranch)),
            elseBranch(std::move(elseBranch)), elifBranches(std::move(elifBranches)) {
            if (!condition || !thenBranch) throw std::runtime_error("IfStmt condition or thenBranch cannot be null");
        }

        void accept(Visitor& visitor) override { visitor.visitIfStmt(this); }
    };

    /// @brief While loop (e.g., while cond { ... })
    class WhileStmt : public Statement {
    public:
        ExprPtr condition;
        StmtPtr body;

        WhileStmt(Token token, ExprPtr condition, StmtPtr body)
            : Statement(token), condition(std::move(condition)), body(std::move(body)) {
            if (!condition || !body) throw std::runtime_error("WhileStmt condition or body cannot be null");
        }

        void accept(Visitor& visitor) override { visitor.visitWhileStmt(this); }
    };

    /// @brief For loop (e.g., for x in iterable { ... })
    class ForStmt : public Statement {
    public:
        std::string variable;
        TypePtr variableType;
        ExprPtr iterable;
        StmtPtr body;

        ForStmt(Token token, std::string variable, TypePtr variableType,
            ExprPtr iterable, StmtPtr body)
            : Statement(token), variable(std::move(variable)), variableType(std::move(variableType)),
            iterable(std::move(iterable)), body(std::move(body)) {
            if (variable.empty()) throw std::runtime_error("ForStmt variable cannot be empty");
            if (!iterable || !body) throw std::runtime_error("ForStmt iterable or body cannot be null");
        }

        void accept(Visitor& visitor) override { visitor.visitForStmt(this); }
    };

    /// @brief Function definition (e.g., def foo(x: int) -> int { ... })
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
            isAsync(isAsync), isPure(isPure) {
            if (name.empty()) throw std::runtime_error("FunctionStmt name cannot be empty");
            if (!body) throw std::runtime_error("FunctionStmt body cannot be null");
        }

        void accept(Visitor& visitor) override { visitor.visitFunctionStmt(this); }
    };

    /// @brief Return statement (e.g., return expr)
    class ReturnStmt : public Statement {
    public:
        ExprPtr value;

        explicit ReturnStmt(Token token, ExprPtr value = nullptr)
            : Statement(token), value(std::move(value)) {}

        void accept(Visitor& visitor) override { visitor.visitReturnStmt(this); }
    };

    /// @brief Class definition (e.g., class Foo { ... })
    class ClassStmt : public Statement {
    public:
        std::string name;
        std::vector<TypePtr> superclasses;
        std::vector<std::shared_ptr<VariableStmt>> fields;
        std::vector<std::shared_ptr<FunctionStmt>> methods;

        ClassStmt(Token token, std::vector<TypePtr> superclasses = {},
            std::vector<std::shared_ptr<VariableStmt>> fields = {},
            std::vector<std::shared_ptr<FunctionStmt>> methods = {})
            : Statement(token), name(token.value), superclasses(std::move(superclasses)),
            fields(std::move(fields)), methods(std::move(methods)) {
            if (name.empty()) throw std::runtime_error("ClassStmt name cannot be empty");
        }

        void accept(Visitor& visitor) override { visitor.visitClassStmt(this); }
    };

    /// @brief Import statement (e.g., import module)
    class ImportStmt : public Statement {
    public:
        std::string module;
        std::vector<std::pair<std::string, std::string>> imports; // original name, alias

        ImportStmt(Token token, std::string module,
            std::vector<std::pair<std::string, std::string>> imports = {})
            : Statement(token), module(std::move(module)), imports(std::move(imports)) {
            if (module.empty()) throw std::runtime_error("ImportStmt module cannot be empty");
        }

        void accept(Visitor& visitor) override { visitor.visitImportStmt(this); }
    };

    /// @brief Case in a match statement
    class MatchCase {
    public:
        ExprPtr pattern;
        StmtPtr body;

        MatchCase(ExprPtr pattern, StmtPtr body)
            : pattern(std::move(pattern)), body(std::move(body)) {
            if (!pattern || !body) throw std::runtime_error("MatchCase pattern or body cannot be null");
        }
    };

    /// @brief Match statement (e.g., match value { case pattern: ... })
    class MatchStmt : public Statement {
    public:
        ExprPtr value;
        std::vector<MatchCase> cases;
        StmtPtr defaultCase;

        MatchStmt(Token token, ExprPtr value, std::vector<MatchCase> cases, StmtPtr defaultCase = nullptr)
            : Statement(token), value(std::move(value)), cases(std::move(cases)),
            defaultCase(std::move(defaultCase)) {
            if (!value) throw std::runtime_error("MatchStmt value cannot be null");
            if (cases.empty()) throw std::runtime_error("MatchStmt must have at least one case");
        }

        void accept(Visitor& visitor) override { visitor.visitMatchStmt(this); }
    };

} // namespace ast