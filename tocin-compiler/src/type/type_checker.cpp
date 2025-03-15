#include "type_checker.h"
#include <sstream>
#include <iostream>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Constructor & Built-in Types Initialization
// ---------------------------------------------------------------------------

TypeChecker::TypeChecker()
{
    // Create a global environment with no enclosing scope.
    currentEnvironment = std::make_shared<Environment>(nullptr);
    currentFunctionReturnType = nullptr; // Ensure this is initialized.
    TypeChecker::initializeBuiltinTypes();
}

void TypeChecker::initializeBuiltinTypes()
{
    // Initialize built-in types (adjust tokens as needed)
    auto intType = std::make_shared<ast::SimpleType>(Token(TokenType::INT, "int", "", 0, 0));
    auto floatType = std::make_shared<ast::SimpleType>(Token(TokenType::FLOAT64, "float64", "", 0, 0));
    auto boolType = std::make_shared<ast::SimpleType>(Token(TokenType::BOOL, "bool", "", 0, 0));
    auto stringType = std::make_shared<ast::SimpleType>(Token(TokenType::STRING, "string", "", 0, 0));
    auto noneType = std::make_shared<ast::SimpleType>(Token(TokenType::IDENTIFIER, "None", "", 0, 0));

    currentEnvironment->define("int", intType, true);
    currentEnvironment->define("float64", floatType, true);
    currentEnvironment->define("bool", boolType, true);
    currentEnvironment->define("string", stringType, true);
    currentEnvironment->define("None", noneType, true);
}

// ---------------------------------------------------------------------------
// Public Interface: analyze and check
// ---------------------------------------------------------------------------

void TypeChecker::analyze(std::shared_ptr<ast::Statement> ast)
{
    // Dummy static analysis: traverse the AST.
    std::cout << "Static analysis: Analysis complete (dummy implementation)." << std::endl;
}

void TypeChecker::check(std::shared_ptr<ast::Statement> ast)
{
    try
    {
        if (ast)
        {
            ast->accept(*this);
        }
        else
        {
            std::cerr << "Type error: Received null AST." << std::endl;
        }
    }
    catch (const TypeCheckError &error)
    {
        std::cerr << "Type error: " << error.what() << std::endl;
    }
}

// ---------------------------------------------------------------------------
// Visitor Methods for Expressions
// ---------------------------------------------------------------------------

void TypeChecker::visitBinaryExpr(ast::BinaryExpr *expr)
{
    expr->left->accept(*this);
    auto leftType = expressionTypes.top();
    expressionTypes.pop();

    expr->right->accept(*this);
    auto rightType = expressionTypes.top();
    expressionTypes.pop();

    // For '+' we support numeric addition or string concatenation.
    if (expr->op.type == TokenType::PLUS)
    {
        if (isNumericType(leftType) && isNumericType(rightType))
        {
            expressionTypes.push(promotedNumericType(leftType, rightType));
        }
        else if (leftType->toString() == "string" && rightType->toString() == "string")
        {
            expressionTypes.push(leftType); // result is a string
        }
        else
        {
            throw error(*expr, "Operator '+' only supports numeric addition or string concatenation");
        }
    }
    // Arithmetic operators (except '+')
    else if (expr->op.type == TokenType::MINUS ||
             expr->op.type == TokenType::STAR ||
             expr->op.type == TokenType::SLASH)
    {

        if (!isNumericType(leftType) || !isNumericType(rightType))
        {
            throw error(*expr, "Arithmetic operations require numeric operands");
        }
        expressionTypes.push(promotedNumericType(leftType, rightType));
    }
    // Comparison operators
    else if (expr->op.type == TokenType::EQUAL_EQUAL ||
             expr->op.type == TokenType::BANG_EQUAL ||
             expr->op.type == TokenType::LESS ||
             expr->op.type == TokenType::LESS_EQUAL ||
             expr->op.type == TokenType::GREATER ||
             expr->op.type == TokenType::GREATER_EQUAL)
    {

        if (!isTypeAssignable(leftType, rightType))
        {
            throw error(*expr, "Comparison requires compatible types");
        }
        // Comparisons produce a boolean.
        expressionTypes.push(std::make_shared<ast::SimpleType>(
            Token(TokenType::BOOL, "bool", "", 0, 0)));
    }
}

void TypeChecker::visitGroupingExpr(ast::GroupingExpr *expr)
{
    // Assume a grouping simply wraps an inner expression.
    if (expr->expression)
    {
        expr->expression->accept(*this);
    }
}

void TypeChecker::visitLiteralExpr(ast::LiteralExpr *expr)
{
    // A simple implementation based on the token type.
    switch (expr->token.type)
    {
    case TokenType::INT:
        expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::INT, "int", "", 0, 0)));
        break;
    case TokenType::FLOAT64:
        expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::FLOAT64, "float64", "", 0, 0)));
        break;
    case TokenType::BOOL:
        expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::BOOL, "bool", "", 0, 0)));
        break;
    case TokenType::STRING:
        expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::STRING, "string", "", 0, 0)));
        break;
    default:
        expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::IDENTIFIER, "None", "", 0, 0)));
        break;
    }
}

void TypeChecker::visitUnaryExpr(ast::UnaryExpr *expr)
{
    // Evaluate the operand.
    expr->right->accept(*this);
    auto type = expressionTypes.top();
    expressionTypes.pop();

    // Validate operand type based on the operator.
    if (expr->op.type == TokenType::MINUS)
    {
        if (!isNumericType(type))
        {
            throw error(*expr, "Unary '-' requires a numeric operand");
        }
    }
    else if (expr->op.type == TokenType::BANG)
    {
        if (type->toString() != "bool")
        {
            throw error(*expr, "Unary '!' requires a boolean operand");
        }
    }

    expressionTypes.push(type);
}

void TypeChecker::visitVariableExpr(ast::VariableExpr *expr)
{
    auto type = currentEnvironment->get(expr->name);
    if (!type)
    {
        throw error(*expr, "Undefined variable '" + expr->name + "'");
    }
    expressionTypes.push(type);
}

void TypeChecker::visitAssignExpr(ast::AssignExpr *expr)
{
    expr->value->accept(*this);
    auto valueType = expressionTypes.top();
    expressionTypes.pop();

    auto varType = currentEnvironment->get(expr->name);
    if (!varType)
    {
        throw error(*expr, "Undefined variable '" + expr->name + "'");
    }

    if (!isTypeAssignable(varType, valueType))
    {
        throw error(*expr, "Cannot assign value of type '" + valueType->toString() +
                               "' to variable of type '" + varType->toString() + "'");
    }

    if (currentEnvironment->isConstant(expr->name))
    {
        throw error(*expr, "Cannot assign to constant variable '" + expr->name + "'");
    }

    expressionTypes.push(varType);
}

void TypeChecker::visitCallExpr(ast::CallExpr *expr)
{
    // A dummy implementation: evaluate the callee and arguments.
    expr->callee->accept(*this);
    for (auto &arg : expr->arguments)
    {
        arg->accept(*this);
    }
    // For now, assume function calls return "None".
    expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::IDENTIFIER, "None", "", 0, 0)));
}

void TypeChecker::visitGetExpr(ast::GetExpr *expr)
{
    // Not implemented: push a default type.
    expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::IDENTIFIER, "None", "", 0, 0)));
}

void TypeChecker::visitSetExpr(ast::SetExpr *expr)
{
    // Not implemented: push a default type.
    expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::IDENTIFIER, "None", "", 0, 0)));
}

void TypeChecker::visitListExpr(ast::ListExpr *expr)
{
    // Not implemented: assume type "list".
    expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::IDENTIFIER, "list", "", 0, 0)));
}

void TypeChecker::visitDictionaryExpr(ast::DictionaryExpr *expr)
{
    // Not implemented: assume type "dict".
    expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::IDENTIFIER, "dict", "", 0, 0)));
}

void TypeChecker::visitLambdaExpr(ast::LambdaExpr *expr)
{
    // Not implemented: assume a lambda type.
    expressionTypes.push(std::make_shared<ast::SimpleType>(Token(TokenType::IDENTIFIER, "lambda", "", 0, 0)));
}

// ---------------------------------------------------------------------------
// Visitor Methods for Statements
// ---------------------------------------------------------------------------

void TypeChecker::visitExpressionStmt(ast::ExpressionStmt *stmt)
{
    stmt->expression->accept(*this);
}

void TypeChecker::visitVariableStmt(ast::VariableStmt *stmt)
{
    // Evaluate initializer and add the variable to the current environment.
    stmt->initializer->accept(*this);
    auto initType = expressionTypes.top();
    expressionTypes.pop();
    currentEnvironment->define(stmt->name, initType, stmt->isConstant);
}

void TypeChecker::visitBlockStmt(ast::BlockStmt *stmt)
{
    beginScope();
    for (auto &statement : stmt->statements)
    {
        statement->accept(*this);
    }
    endScope();
}

void TypeChecker::visitIfStmt(ast::IfStmt *stmt)
{
    stmt->condition->accept(*this);
    auto condType = expressionTypes.top();
    expressionTypes.pop();
    if (condType->toString() != "bool")
    {
        throw error(*stmt, "If statement condition must be a boolean");
    }
    stmt->thenBranch->accept(*this);
    if (stmt->elseBranch)
    {
        stmt->elseBranch->accept(*this);
    }
}

void TypeChecker::visitWhileStmt(ast::WhileStmt *stmt)
{
    stmt->condition->accept(*this);
    auto condType = expressionTypes.top();
    expressionTypes.pop();
    if (condType->toString() != "bool")
    {
        throw error(*stmt, "While statement condition must be a boolean");
    }
    stmt->body->accept(*this);
}

void TypeChecker::visitForStmt(ast::ForStmt *stmt)
{
    beginScope();
    // Evaluate the iterable.
    stmt->iterable->accept(*this);
    // In a real implementation, determine the element type.
    currentEnvironment->define(stmt->variable, std::make_shared<ast::SimpleType>(Token(TokenType::IDENTIFIER, "var", "", 0, 0)), false);
    stmt->body->accept(*this);
    endScope();
}

void TypeChecker::visitFunctionStmt(ast::FunctionStmt *stmt)
{
    // Register the function name in the current environment (allows recursion).
    currentEnvironment->define(stmt->name, stmt->returnType, true);

    beginScope();
    // Define parameters in the new scope.
    for (const auto &param : stmt->parameters)
    {
        currentEnvironment->define(param.name, param.type, false);
    }
    // Save the previous return type.
    auto previousReturnType = currentFunctionReturnType;
    currentFunctionReturnType = stmt->returnType;
    stmt->body->accept(*this);
    currentFunctionReturnType = previousReturnType;
    endScope();
}

void TypeChecker::visitReturnStmt(ast::ReturnStmt *stmt)
{
    if (!currentFunctionReturnType)
    {
        throw error(*stmt, "Return statement outside of function");
    }
    if (stmt->value)
    {
        stmt->value->accept(*this);
        auto returnedType = expressionTypes.top();
        expressionTypes.pop();
        if (!isTypeAssignable(currentFunctionReturnType, returnedType))
        {
            throw error(*stmt, "Cannot return value of type '" + returnedType->toString() +
                                   "' from function returning '" + currentFunctionReturnType->toString() + "'");
        }
    }
    else if (currentFunctionReturnType->toString() != "None")
    {
        throw error(*stmt, "Function must return a value of type '" +
                               currentFunctionReturnType->toString() + "'");
    }
}

void TypeChecker::visitClassStmt(ast::ClassStmt *stmt)
{
    // Not implemented.
}

void TypeChecker::visitImportStmt(ast::ImportStmt *stmt)
{
    // Not implemented.
}

void TypeChecker::visitMatchStmt(ast::MatchStmt *stmt)
{
    // Not implemented.
}

// ---------------------------------------------------------------------------
// Scope Management
// ---------------------------------------------------------------------------

void TypeChecker::beginScope()
{
    // Create a new environment with the current one as its parent.
    currentEnvironment = std::make_shared<Environment>(currentEnvironment);
}

void TypeChecker::endScope()
{
    // Pop back to the enclosing environment using the getter.
    if (currentEnvironment && currentEnvironment->getEnclosing())
    {
        currentEnvironment = currentEnvironment->getEnclosing();
    }
    else
    {
        // Already at global scope; optionally handle this case.
    }
}

// ---------------------------------------------------------------------------
// Utility Functions
// ---------------------------------------------------------------------------

std::shared_ptr<ast::Type> TypeChecker::checkExpression(std::shared_ptr<ast::Expression> expr)
{
    expr->accept(*this);
    auto result = expressionTypes.top();
    expressionTypes.pop();
    return result;
}

bool TypeChecker::isTypeAssignable(std::shared_ptr<ast::Type> target, std::shared_ptr<ast::Type> source)
{
    if (target->toString() == source->toString())
    {
        return true;
    }
    // Allow int-to-float64 promotion.
    if (isNumericType(target) && isNumericType(source))
    {
        return target->toString() == "float64" && source->toString() == "int";
    }
    return false;
}

bool TypeChecker::isNumericType(std::shared_ptr<ast::Type> type)
{
    auto typeName = type->toString();
    return typeName == "int" || typeName == "float64";
}

std::shared_ptr<ast::Type> TypeChecker::promotedNumericType(std::shared_ptr<ast::Type> type1, std::shared_ptr<ast::Type> type2)
{
    if (type1->toString() == "float64" || type2->toString() == "float64")
    {
        return std::make_shared<ast::SimpleType>(Token(TokenType::FLOAT64, "float64", "", 0, 0));
    }
    return std::make_shared<ast::SimpleType>(Token(TokenType::INT, "int", "", 0, 0));
}

TypeCheckError TypeChecker::error(const ast::Node &node, const std::string &message)
{
    std::stringstream ss;
    ss << node.token.filename << ":" << node.token.line << ":" << node.token.column
       << ": " << message;
    return TypeCheckError(ss.str());
}

// ---------------------------------------------------------------------------
// Environment Methods
// ---------------------------------------------------------------------------

void TypeChecker::Environment::define(const std::string &name, std::shared_ptr<ast::Type> type, bool isConst)
{
    values[name] = type;
    constants[name] = isConst;
}

std::shared_ptr<ast::Type> TypeChecker::Environment::get(const std::string &name)
{
    auto it = values.find(name);
    if (it != values.end())
    {
        return it->second;
    }
    if (enclosing)
    {
        return enclosing->get(name);
    }
    return nullptr;
}

bool TypeChecker::Environment::isConstant(const std::string &name)
{
    auto it = constants.find(name);
    if (it != constants.end())
    {
        return it->second;
    }
    if (enclosing)
    {
        return enclosing->isConstant(name);
    }
    return false;
}
