#pragma once

#include "../pch.h"
#include "../ast/ast.h"
#include "../error/error_handler.h"

namespace type_checker
{

    /**
     * @brief Class for managing extension functions
     *
     * Extension functions allow adding methods to existing types without
     * modifying the original type, similar to Kotlin's extension functions.
     */
    class ExtensionManager
    {
    public:
        ExtensionManager(error::ErrorHandler &errorHandler)
            : errorHandler(errorHandler) {}

        /**
         * @brief Register an extension function for a type
         *
         * @param targetType The type being extended
         * @param functionName The name of the extension function
         * @param function The function being registered
         * @return true if registration succeeded
         */
        bool registerExtension(ast::TypePtr targetType, const std::string &functionName,
                               ast::FunctionStmt *function)
        {
            std::string typeName = targetType->toString();

            // Check if this extension already exists
            auto &extensionsForType = extensions[typeName];
            if (extensionsForType.find(functionName) != extensionsForType.end())
            {
                errorHandler.reportError(
                    error::ErrorCode::T003_UNDEFINED_FUNCTION,
                    "Extension function '" + functionName + "' already defined for type '" + typeName + "'",
                    "", 0, 0, error::ErrorSeverity::ERROR);
                return false;
            }

            // Register the extension
            extensionsForType[functionName] = function;

            return true;
        }

        /**
         * @brief Look up an extension function for a type
         *
         * @param targetType The type being extended
         * @param functionName The name of the extension function
         * @return ast::FunctionStmt* The function, or nullptr if not found
         */
        ast::FunctionStmt *findExtension(ast::TypePtr targetType, const std::string &functionName)
        {
            std::string typeName = targetType->toString();

            // Look for extensions for this exact type
            auto typeIt = extensions.find(typeName);
            if (typeIt != extensions.end())
            {
                auto funcIt = typeIt->second.find(functionName);
                if (funcIt != typeIt->second.end())
                {
                    return funcIt->second;
                }
            }

            // Check for interface/trait implementations
            if (auto genericType = std::dynamic_pointer_cast<ast::GenericType>(targetType))
            {
                // Check if this is a generic type that might have trait bounds
                for (const auto& typeArg : genericType->typeArguments)
                {
                    if (auto typeArgExtension = findExtension(typeArg, functionName))
                    {
                        return typeArgExtension;
                    }
                }
            }

            // Check for built-in type extensions (e.g., String, Array, etc.)
            if (auto simpleType = std::dynamic_pointer_cast<ast::SimpleType>(targetType))
            {
                // Check for extensions on primitive types
                std::string primitiveName = simpleType->toString();
                if (primitiveName == "string" || primitiveName == "int" || 
                    primitiveName == "float" || primitiveName == "bool")
                {
                    auto primitiveIt = extensions.find(primitiveName);
                    if (primitiveIt != extensions.end())
                    {
                        auto funcIt = primitiveIt->second.find(functionName);
                        if (funcIt != primitiveIt->second.end())
                        {
                            return funcIt->second;
                        }
                    }
                }
            }

            // Check for class inheritance (simplified - would need proper inheritance tracking)
            if (auto classType = std::dynamic_pointer_cast<ast::ClassType>(targetType))
            {
                // For now, just check if there are any extensions registered for "Object" or base types
                // In a real implementation, this would traverse the inheritance hierarchy
                auto objectIt = extensions.find("Object");
                if (objectIt != extensions.end())
                {
                    auto funcIt = objectIt->second.find(functionName);
                    if (funcIt != objectIt->second.end())
                    {
                        return funcIt->second;
                    }
                }
            }

            return nullptr;
        }

        /**
         * @brief Get all extension functions for a type
         *
         * @param targetType The type to find extensions for
         * @return std::vector<std::string> Names of available extension functions
         */
        std::vector<std::string> getExtensionsForType(ast::TypePtr targetType)
        {
            std::string typeName = targetType->toString();
            std::vector<std::string> result;

            auto typeIt = extensions.find(typeName);
            if (typeIt != extensions.end())
            {
                for (const auto &[name, func] : typeIt->second)
                {
                    result.push_back(name);
                }
            }

            return result;
        }

    private:
        // Map of type name -> (function name -> function)
        std::map<std::string, std::map<std::string, ast::FunctionStmt *>> extensions;
        error::ErrorHandler &errorHandler;
    };

    /**
     * @brief AST node for an extension function declaration
     *
     * Represents a function that extends an existing type, similar to
     * Kotlin's extension functions.
     */
    class ExtensionFunctionStmt : public ast::FunctionStmt
    {
    public:
        ExtensionFunctionStmt(const lexer::Token &token, const std::string &name,
                              std::vector<ast::Parameter> parameters,
                              ast::TypePtr returnType,
                              ast::StmtPtr body)
            : ast::FunctionStmt(token, name, parameters, returnType, body, false),
              extendedType("") {}

        ExtensionFunctionStmt(const lexer::Token &token, const std::string &name,
                              std::vector<ast::Parameter> parameters,
                              ast::TypePtr returnType,
                              ast::StmtPtr body,
                              const std::string &extendedType)
            : ast::FunctionStmt(token, name, parameters, returnType, body, false),
              extendedType(extendedType) {}

        virtual void accept(ast::Visitor &visitor) override
        {
            visitor.visitFunctionStmt(this); // No cast needed now
        }

        ast::FunctionStmt *toRegularFunction()
        {
            lexer::Token token = this->token; // Use existing token
            ast::Parameter thisParam(token, "this", std::make_shared<ast::ClassType>(token, extendedType));
            std::vector<ast::Parameter> allParams = {thisParam};
            allParams.insert(allParams.end(), parameters.begin(), parameters.end());
            return new ast::FunctionStmt(token, name, allParams, returnType, body, false);
        }

        const std::string &getExtendedType() const { return extendedType; }

    private:
        std::string extendedType;
    };

    /**
     * @brief AST node for an extension call expression
     *
     * Represents a call to an extension function. This is transformed during
     * code generation into a call to the regular function with the target
     * object passed as the first argument.
     */
    class ExtensionCallExpr : public ast::CallExpr
    {
    public:
        ExtensionCallExpr(const lexer::Token &token, ast::ExprPtr target,
                          std::string functionName,
                          std::vector<ast::ExprPtr> arguments)
            : CallExpr(token, target, arguments), functionName(functionName) {}

        virtual void accept(ast::Visitor &visitor) override
        {
            visitor.visitCallExpr(this);
        }

        virtual ast::TypePtr getType() const override
        {
            return nullptr; // This should be set during type checking
        }

        ast::CallExpr *toRegularCall()
        {
            lexer::Token token = this->token; // Use existing token
            std::string fullName = callee->getType()->toString() + "_" + functionName;
            auto funcExpr = std::make_shared<ast::VariableExpr>(token, fullName);
            std::vector<ast::ExprPtr> allArgs = {callee};
            allArgs.insert(allArgs.end(), arguments.begin(), arguments.end());
            return new ast::CallExpr(token, funcExpr, allArgs);
        }

        const std::string &getFunctionName() const { return functionName; }

    private:
        std::string functionName;
    };

} // namespace type_checker
