#pragma once

#include "../pch.h"
#include "../ast/ast.h"
#include "../ast/match_stmt.h"
#include "../error/error_handler.h"
#include "../lexer/token.h"

namespace type_checker
{

    /**
     * @brief Enum representing the variants of Option<T>
     *
     * Similar to Rust's Option, this represents either Some(value) or None
     */
    enum class OptionVariant
    {
        SOME,
        NONE
    };

    /**
     * @brief Enum representing the variants of Result<T, E>
     *
     * Similar to Rust's Result, this represents either Ok(value) or Err(error)
     */
    enum class ResultVariant
    {
        OK,
        ERR
    };

    /**
     * @brief Class for checking and verifying Option<T> types
     */
    class OptionType
    {
    public:
        static const std::string TYPE_NAME;

        /**
         * @brief Check if a type is an Option type
         *
         * @param type The type to check
         * @return true if the type is an Option
         */
        static bool isOptionType(ast::TypePtr type)
        {
            if (auto genericType = std::dynamic_pointer_cast<ast::GenericType>(type))
            {
                return genericType->name == TYPE_NAME;
            }
            return false;
        }

        /**
         * @brief Create an Option type with the given value type
         *
         * @param valueType The type parameter for Option<T>
         * @return ast::TypePtr The Option type
         */
        static ast::TypePtr createOptionType(ast::TypePtr valueType)
        {
            lexer::Token token; // Create a default token
            return std::make_shared<ast::GenericType>(token, TYPE_NAME, std::vector<ast::TypePtr>{valueType});
        }

        /**
         * @brief Extract the value type from an Option type
         *
         * @param optionType The Option type
         * @return ast::TypePtr The value type T in Option<T>
         */
        static ast::TypePtr getValueType(ast::TypePtr optionType)
        {
            if (auto genericType = std::dynamic_pointer_cast<ast::GenericType>(optionType))
            {
                if (genericType->name == TYPE_NAME && !genericType->typeArguments.empty())
                {
                    return genericType->typeArguments[0];
                }
            }
            return nullptr;
        }
    };

    /**
     * @brief Class for checking and verifying Result<T, E> types
     */
    class ResultType
    {
    public:
        static const std::string TYPE_NAME;

        /**
         * @brief Check if a type is a Result type
         *
         * @param type The type to check
         * @return true if the type is a Result
         */
        static bool isResultType(ast::TypePtr type)
        {
            if (auto genericType = std::dynamic_pointer_cast<ast::GenericType>(type))
            {
                return genericType->name == TYPE_NAME;
            }
            return false;
        }

        /**
         * @brief Create a Result type with the given value and error types
         *
         * @param valueType The Ok value type parameter for Result<T, E>
         * @param errorType The Err error type parameter for Result<T, E>
         * @return ast::TypePtr The Result type
         */
        static ast::TypePtr createResultType(ast::TypePtr valueType, ast::TypePtr errorType)
        {
            lexer::Token token; // Create a default token
            return std::make_shared<ast::GenericType>(
                token,
                TYPE_NAME,
                std::vector<ast::TypePtr>{valueType, errorType});
        }

        /**
         * @brief Extract the value type from a Result type
         *
         * @param resultType The Result type
         * @return ast::TypePtr The value type T in Result<T, E>
         */
        static ast::TypePtr getValueType(ast::TypePtr resultType)
        {
            if (auto genericType = std::dynamic_pointer_cast<ast::GenericType>(resultType))
            {
                if (genericType->name == TYPE_NAME && genericType->typeArguments.size() >= 1)
                {
                    return genericType->typeArguments[0];
                }
            }
            return nullptr;
        }

        /**
         * @brief Extract the error type from a Result type
         *
         * @param resultType The Result type
         * @return ast::TypePtr The error type E in Result<T, E>
         */
        static ast::TypePtr getErrorType(ast::TypePtr resultType)
        {
            if (auto genericType = std::dynamic_pointer_cast<ast::GenericType>(resultType))
            {
                if (genericType->name == TYPE_NAME && genericType->typeArguments.size() >= 2)
                {
                    return genericType->typeArguments[1];
                }
            }
            return nullptr;
        }
    };

    // Define the static const members
    // const std::string OptionType::TYPE_NAME = "Option";
    // const std::string ResultType::TYPE_NAME = "Result";

    /**
     * @brief Class for pattern matching on Option and Result types
     */
    class ResultOptionMatcher
    {
    public:
        ResultOptionMatcher(error::ErrorHandler &errorHandler)
            : errorHandler(errorHandler) {}

        /**
         * @brief Check if pattern matching on an Option type is valid
         *
         * @param matchType The type being matched (Option<T>)
         * @param patterns The patterns used in the match expression
         * @return true if the match is valid
         */
        bool checkOptionMatch(ast::TypePtr matchType, const std::vector<ast::PatternPtr> &patterns)
        {
            if (!OptionType::isOptionType(matchType))
            {
                errorHandler.reportError(
                    error::ErrorCode::T001_TYPE_MISMATCH,
                    "Cannot match on non-Option type",
                    "", 0, 0, error::ErrorSeverity::ERROR);
                return false;
            }

            ast::TypePtr valueType = OptionType::getValueType(matchType);
            bool hasSome = false;
            bool hasNone = false;

            for (const auto &pattern : patterns)
            {
                // Check for Some(x) pattern
                if (auto constructorPattern = std::dynamic_pointer_cast<ast::ConstructorPattern>(pattern))
                {
                    if (constructorPattern->getName() == "Some")
                    {
                        hasSome = true;
                        // Check that the inner pattern matches the value type
                        if (constructorPattern->getArguments().size() != 1)
                        {
                            errorHandler.reportError(
                                error::ErrorCode::T033_INCORRECT_ARGUMENT_COUNT,
                                "Option::Some expects exactly 1 argument",
                                "", 0, 0, error::ErrorSeverity::ERROR);
                            return false;
                        }
                        if (patterns.size() > 1)
                        {
                            errorHandler.reportError(
                                error::ErrorCode::T033_INCORRECT_ARGUMENT_COUNT,
                                "Option::Some expects exactly 1 argument",
                                "", 0, 0, error::ErrorSeverity::ERROR);
                            return false;
                        }
                        // Check that the pattern matches the inner type
                        auto innerType = std::dynamic_pointer_cast<ast::GenericType>(valueType);
                        if (innerType && !innerType->typeArguments.empty())
                        {
                            auto expectedType = innerType->typeArguments[0];
                            // Note: Pattern doesn't have getType() method, this needs to be implemented
                            // For now, we'll skip this check
                            // auto patternType = patterns[0]->getType();
                            // if (!typeChecker.typesCompatible(patternType, expectedType))
                            // {
                            //     errorHandler.reportError(
                            //         error::ErrorCode::T001_TYPE_MISMATCH,
                            //         "Pattern type does not match Option inner type",
                            //         "", 0, 0, error::ErrorSeverity::ERROR);
                            //     return false;
                            // }
                        }
                        return true;
                    }
                    else if (constructorPattern->getName() == "None")
                    {
                        hasNone = true;
                        // Check that None has no arguments
                        if (!constructorPattern->getArguments().empty())
                        {
                            errorHandler.reportError(
                                error::ErrorCode::T033_INCORRECT_ARGUMENT_COUNT,
                                "Option::None does not take any arguments",
                                "", 0, 0, error::ErrorSeverity::ERROR);
                            return false;
                        }
                    }
                    else
                    {
                        errorHandler.reportError(
                            error::ErrorCode::T001_TYPE_MISMATCH,
                            "Invalid Option constructor: " + constructorPattern->getName(),
                            "", 0, 0, error::ErrorSeverity::ERROR);
                        return false;
                    }
                }
                // Allow wildcard pattern
                else if (std::dynamic_pointer_cast<ast::WildcardPattern>(pattern))
                {
                    // Wildcard is always valid
                    continue;
                }
            }

            // Check exhaustiveness
            if (!hasSome || !hasNone)
            {
                errorHandler.reportError(
                    error::ErrorCode::P001_NON_EXHAUSTIVE_PATTERNS,
                    "Non-exhaustive patterns: Option match must handle both Some and None cases",
                    "", 0, 0, error::ErrorSeverity::ERROR);
                return false;
            }

            return true;
        }

        /**
         * @brief Check if pattern matching on a Result type is valid
         *
         * @param matchType The type being matched (Result<T, E>)
         * @param patterns The patterns used in the match expression
         * @return true if the match is valid
         */
        bool checkResultMatch(ast::TypePtr matchType, const std::vector<ast::PatternPtr> &patterns)
        {
            if (!ResultType::isResultType(matchType))
            {
                errorHandler.reportError(
                    error::ErrorCode::T001_TYPE_MISMATCH,
                    "Cannot match on non-Result type",
                    "", 0, 0, error::ErrorSeverity::ERROR);
                return false;
            }

            ast::TypePtr valueType = ResultType::getValueType(matchType);
            ast::TypePtr errorType = ResultType::getErrorType(matchType);
            bool hasOk = false;
            bool hasErr = false;

            for (const auto &pattern : patterns)
            {
                // Check for Ok(x) and Err(e) patterns
                if (auto constructorPattern = std::dynamic_pointer_cast<ast::ConstructorPattern>(pattern))
                {
                    if (constructorPattern->getName() == "Ok")
                    {
                        hasOk = true;
                        // Check that Ok has exactly one argument
                        if (constructorPattern->getArguments().size() != 1)
                        {
                            errorHandler.reportError(
                                error::ErrorCode::T033_INCORRECT_ARGUMENT_COUNT,
                                "Result::Ok expects exactly 1 argument",
                                "", 0, 0, error::ErrorSeverity::ERROR);
                            return false;
                        }
                    }
                    else if (constructorPattern->getName() == "Err")
                    {
                        hasErr = true;
                        // Check that Err has exactly one argument
                        if (constructorPattern->getArguments().size() != 1)
                        {
                            errorHandler.reportError(
                                error::ErrorCode::T033_INCORRECT_ARGUMENT_COUNT,
                                "Result::Err expects exactly 1 argument",
                                "", 0, 0, error::ErrorSeverity::ERROR);
                            return false;
                        }
                    }
                }
                // Allow wildcard pattern
                else if (std::dynamic_pointer_cast<ast::WildcardPattern>(pattern))
                {
                    // Wildcard is always valid
                    continue;
                }
            }

            // Check exhaustiveness
            if (!hasOk || !hasErr)
            {
                errorHandler.reportError(
                    error::ErrorCode::P001_NON_EXHAUSTIVE_PATTERNS,
                    "Non-exhaustive patterns: Result match must handle both Ok and Err cases",
                    "", 0, 0, error::ErrorSeverity::ERROR);
                return false;
            }

            return true;
        }

    private:
        error::ErrorHandler &errorHandler;
    };

} // namespace type_checker
