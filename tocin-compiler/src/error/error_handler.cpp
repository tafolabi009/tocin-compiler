

#include "error_handler.h"
#include <iostream>
#include <sstream>

namespace error {

    std::string CompilerError::formattedMessage() const {
        std::stringstream ss;
        ss << filename << ":" << line << ":" << column << ": ";

        switch (severity) {
        case ErrorSeverity::INFO:
            ss << "info: ";
            break;
        case ErrorSeverity::WARNING:
            ss << "warning: ";
            break;
        case ErrorSeverity::ERROR:
            ss << "error: ";
            break;
        case ErrorSeverity::FATAL:
            ss << "fatal error: ";
            break;
        }

        ss << what();
        return ss.str();
    }

    ErrorHandler::ErrorHandler()
        : recovering(false), errorCount(0), warningCount(0) {}

    void ErrorHandler::reportError(const CompilerError& error) {
        errors.push_back(error);

        if (error.severity == ErrorSeverity::WARNING) {
            warningCount++;
        }
        else if (error.severity >= ErrorSeverity::ERROR) {
            errorCount++;
        }

        logError(error);
    }

    void ErrorHandler::reportError(
        const std::string& message,
        const std::string& filename,
        int line,
        int column,
        ErrorSeverity severity) {

        reportError(CompilerError(message, filename, line, column, severity));
    }

    void ErrorHandler::reportError(
        const std::string& message,
        const Token& token,
        ErrorSeverity severity) {

        reportError(message, token.filename, token.line, token.column, severity);
    }

    bool ErrorHandler::hasErrors() const {
        return errorCount > 0;
    }

    bool ErrorHandler::hasWarnings() const {
        return warningCount > 0;
    }

    void ErrorHandler::clear() {
        errors.clear();
        errorCount = 0;
        warningCount = 0;
        recovering = false;
    }

    const std::vector<CompilerError>& ErrorHandler::getErrors() const {
        return errors;
    }

    void ErrorHandler::beginRecovery() {
        recovering = true;
    }

    void ErrorHandler::endRecovery() {
        recovering = false;
    }

    bool ErrorHandler::isRecovering() const {
        return recovering;
    }

    void ErrorHandler::logError(const CompilerError& error) {
        std::string message = error.formattedMessage();

        if (error.severity >= ErrorSeverity::ERROR) {
            std::cerr << message << std::endl;
        }
        else {
            std::cout << message << std::endl;
        }
    }

} // namespace error