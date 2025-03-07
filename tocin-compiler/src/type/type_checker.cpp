#include "type_checker.h"
#include <sstream>
#include <iostream>

TypeChecker::TypeChecker() {
    currentEnvironment = std::make_shared<Environment>();
    initializeBuiltinTypes();
}

void TypeChecker::initializeBuiltinTypes() {
    // Initialize built-in types
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

void TypeChecker::analyze(std::shared_ptr<ast::Statement> ast) {
    // For demonstration: check for functions with non-None return type that lack a return statement.
    // (A real implementation would traverse the AST more thoroughly.)
    std::cout << "Static analysis: Analysis complete (dummy implementation)." << std::endl;
}

void TypeChecker::check(std::shared_ptr<ast::Statement> ast) {
    try {
        ast->accept(*this);
    }
    catch (const TypeCheckError& error) {
        std::cerr << "Type error: " << error.what() << std::endl;
    }
}

void TypeChecker::visitBinaryExpr(ast::BinaryExpr* expr) {
    expr->left->accept(*this);
    auto leftType = expressionTypes.top();
    expressionTypes.pop();

    expr->right->accept(*this);
    auto rightType = expressionTypes.top();
    expressionTypes.pop();

    // Type checking rules for binary operations
    if (expr->op.type == TokenType::PLUS ||
        expr->op.type == TokenType::MINUS ||
        expr->op.type == TokenType::STAR ||
        expr->op.type == TokenType::SLASH) {

        if (!isNumericType(leftType) || !isNumericType(rightType)) {
            throw error(*expr, "Arithmetic operations require numeric operands");
        }
        // Result type is float64 if either operand is float64, otherwise int
        expressionTypes.push(promotedNumericType(leftType, rightType));
    }
    else if (expr->op.type == TokenType::EQUAL_EQUAL ||
        expr->op.type == TokenType::BANG_EQUAL ||
        expr->op.type == TokenType::LESS ||
        expr->op.type == TokenType::LESS_EQUAL ||
        expr->op.type == TokenType::GREATER ||
        expr->op.type == TokenType::GREATER_EQUAL) {

        if (!isTypeAssignable(leftType, rightType)) {
            throw error(*expr, "Comparison requires compatible types");
        }
        // Comparison operations always result in bool
        expressionTypes.push(std::make_shared<ast::SimpleType>(
            Token(TokenType::BOOL, "bool", "", 0, 0)));
    }
}

void TypeChecker::visitVariableExpr(ast::VariableExpr* expr) {
    auto type = currentEnvironment->get(expr->name);
    if (!type) {
        throw error(*expr, "Undefined variable '" + expr->name + "'");
    }
    expressionTypes.push(type);
}

void TypeChecker::visitAssignExpr(ast::AssignExpr* expr) {
    expr->value->accept(*this);
    auto valueType = expressionTypes.top();
    expressionTypes.pop();

    auto varType = currentEnvironment->get(expr->name);
    if (!varType) {
        throw error(*expr, "Undefined variable '" + expr->name + "'");
    }

    if (!isTypeAssignable(varType, valueType)) {
        throw error(*expr, "Cannot assign value of type '" + valueType->toString() +
            "' to variable of type '" + varType->toString() + "'");
    }

    if (currentEnvironment->isConstant(expr->name)) {
        throw error(*expr, "Cannot assign to constant variable '" + expr->name + "'");
    }

    expressionTypes.push(varType);
}

void TypeChecker::visitFunctionStmt(ast::FunctionStmt* stmt) {
    beginScope();

    // Add parameters to the new scope
    for (const auto& param : stmt->parameters) {
        currentEnvironment->define(param.name, param.type, false);
    }

    // Set current function return type for checking return statements
    auto previousReturnType = currentFunctionReturnType;
    currentFunctionReturnType = stmt->returnType;

    // Check function body
    stmt->body->accept(*this);

    // Restore previous return type
    currentFunctionReturnType = previousReturnType;

    endScope();
}

void TypeChecker::visitReturnStmt(ast::ReturnStmt* stmt) {
    if (!currentFunctionReturnType) {
        throw error(*stmt, "Return statement outside of function");
    }

    if (stmt->value) {
        stmt->value->accept(*this);
        auto returnedType = expressionTypes.top();
        expressionTypes.pop();

        if (!isTypeAssignable(currentFunctionReturnType, returnedType)) {
            throw error(*stmt, "Cannot return value of type '" + returnedType->toString() +
                "' from function returning '" + currentFunctionReturnType->toString() + "'");
        }
    }
    else if (currentFunctionReturnType->toString() != "None") {
        throw error(*stmt, "Function must return a value of type '" +
            currentFunctionReturnType->toString() + "'");
    }
}

bool TypeChecker::isTypeAssignable(std::shared_ptr<ast::Type> target,
    std::shared_ptr<ast::Type> source) {
    if (target->toString() == source->toString()) {
        return true;
    }

    // Handle numeric type promotions
    if (isNumericType(target) && isNumericType(source)) {
        // Allow int to float64 promotion
        return target->toString() == "float64" && source->toString() == "int";
    }

    return false;
}

bool TypeChecker::isNumericType(std::shared_ptr<ast::Type> type) {
    auto typeName = type->toString();
    return typeName == "int" || typeName == "float64";
}

std::shared_ptr<ast::Type> TypeChecker::promotedNumericType(
    std::shared_ptr<ast::Type> type1,
    std::shared_ptr<ast::Type> type2) {

    if (type1->toString() == "float64" || type2->toString() == "float64") {
        return std::make_shared<ast::SimpleType>(
            Token(TokenType::FLOAT64, "float64", "", 0, 0));
    }
    return std::make_shared<ast::SimpleType>(
        Token(TokenType::INT, "int", "", 0, 0));
}

TypeCheckError TypeChecker::error(const ast::Node& node, const std::string& message) {
    std::stringstream ss;
    ss << node.token.filename << ":" << node.token.line << ":" << node.token.column
        << ": " << message;
    return TypeCheckError(ss.str());
}

void TypeChecker::Environment::define(const std::string& name,
    std::shared_ptr<ast::Type> type,
    bool isConst) {
    values[name] = type;
    constants[name] = isConst;
}

std::shared_ptr<ast::Type> TypeChecker::Environment::get(const std::string& name) {
    auto it = values.find(name);
    if (it != values.end()) {
        return it->second;
    }
    if (enclosing) {
        return enclosing->get(name);
    }
    return nullptr;
}

bool TypeChecker::Environment::isConstant(const std::string& name) {
    auto it = constants.find(name);
    if (it != constants.end()) {
        return it->second;
    }
    if (enclosing) {
        return enclosing->isConstant(name);
    }
    return false;
}