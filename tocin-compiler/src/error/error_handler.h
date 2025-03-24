// File: src/error/error_handler.h
#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <stdexcept>
#include "../lexer/token.h"

namespace error {
    enum class ErrorSeverity { INFO, WARNING, ERROR, FATAL };

    class CompilerError : public std::exception {
    public:
        CompilerError(const std::string& message, const std::string& filename, int line, int column, ErrorSeverity severity)
            : message(message), filename(filename), line(line), column(column), severity(severity) {}

        const char* what() const noexcept override { return message.c_str(); }
        std::string formattedMessage() const;

        std::string message;
        std::string filename;
        int line;
        int column;
        ErrorSeverity severity;
    };

    class ErrorHandler {
    public:
        ErrorHandler();

        void reportError(const CompilerError& error);
        void reportError(
            const std::string& message,
            const std::string& filename,
            int line,
            int column,
            ErrorSeverity severity);
        void reportError(
            const std::string& message,
            const Token& token,
            ErrorSeverity severity);

        bool hasErrors() const;
        bool hasWarnings() const;
        void clear();
        const std::vector<CompilerError>& getErrors() const;

        void beginRecovery();
        void endRecovery();
        bool isRecovering() const;

    private:
        void logError(const CompilerError& error);

        std::vector<CompilerError> errors;
        bool recovering;
        int errorCount;
        int warningCount;
    };
}

#endif
