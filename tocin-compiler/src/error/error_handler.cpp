// File: src/error/error_handler.cpp
#include "../error/error_handler.h"
#include <iostream>

namespace error {

    void ErrorHandler::reportError(const std::string& message, const Token& token, ErrorSeverity severity) {
        reportError(message, token.filename, token.line, token.column, severity);
    }

    void ErrorHandler::reportError(const std::string& message, const std::string& filename, int line, int column,
        ErrorSeverity severity) {
        std::lock_guard<std::mutex> lock(mutex);
        if (inRecovery) return;

        errors.emplace_back(message, filename, line, column, severity);
        std::string severityStr = (severity == ErrorSeverity::WARNING) ? "Warning" :
            (severity == ErrorSeverity::ERROR) ? "Error" : "Fatal";
        std::cerr << filename << ":" << line << ":" << column << ": " << severityStr << ": " << message << std::endl;

        if (severity == ErrorSeverity::FATAL) {
            throw std::runtime_error("Fatal error encountered: " + message);
        }
    }

    void ErrorHandler::beginRecovery() {
        std::lock_guard<std::mutex> lock(mutex);
        inRecovery = true;
    }

    void ErrorHandler::endRecovery() {
        std::lock_guard<std::mutex> lock(mutex);
        inRecovery = false;
    }

    bool ErrorHandler::hasErrors() const {
        std::lock_guard<std::mutex> lock(mutex);
        for (const auto& err : errors) {
            if (err.severity != ErrorSeverity::WARNING) return true;
        }
        return false;
    }

} // namespace error