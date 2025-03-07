#pragma once

#include <string>
#include <vector>
#include <memory>
#include <exception>
#include "../lexer/token.h"

namespace error {

    enum class ErrorSeverity {
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    class CompilerError : public std::runtime_error {
    public:
        CompilerError(const std::string& message,
            const std::string& filename,
            int line,
            int column,
            ErrorSeverity severity)
            : std::runtime_error(message),
            filename(filename),
            line(line),
            column(column),
            severity(severity) {}

        std::string filename;
        int line;
        int column;
        ErrorSeverity severity;

        std::string formattedMessage() const;
    };

    class ErrorHandler {
    public:
        ErrorHandler();
        ~ErrorHandler() = default;

        void reportError(const CompilerError& error);
        void reportError(const std::string& message,
            const std::string& filename,
            int line,
            int column,
            ErrorSeverity severity = ErrorSeverity::ERROR);

        void reportError(const std::string& message,
            const Token& token,
            ErrorSeverity severity = ErrorSeverity::ERROR);

        bool hasErrors() const;
        bool hasWarnings() const;
        void clear();

        // Get all errors and warnings
        const std::vector<CompilerError>& getErrors() const;

        // Recovery mechanisms
        void beginRecovery();
        void endRecovery();
        bool isRecovering() const;

    private:
        std::vector<CompilerError> errors;
        bool recovering;
        int errorCount;
        int warningCount;

        void logError(const CompilerError& error);
    };

} // namespace error