// File: src/error/error_handler.h
#pragma once

#include <string>
#include <vector>
#include "../lexer/token.h"
#include <mutex>

namespace error {

    /// @brief Severity levels for errors
    enum class ErrorSeverity {
        WARNING,
        ERROR,
        FATAL
    };

    /// @brief Represents a compilation error or warning
    struct Error {
        std::string message;         ///< Error message
        std::string filename;        ///< Source file
        int line;                    ///< Line number
        int column;                  ///< Column number
        ErrorSeverity severity;      ///< Severity level

        Error(const std::string& msg, const std::string& fname, int l, int c, ErrorSeverity sev)
            : message(msg), filename(fname), line(l), column(c), severity(sev) {}
    };

    /// @brief Manages error reporting and recovery
    class ErrorHandler {
    public:
        /// @brief Reports an error with token information
        void reportError(const std::string& message, const Token& token, ErrorSeverity severity);

        /// @brief Reports an error with explicit location
        void reportError(const std::string& message, const std::string& filename, int line, int column,
            ErrorSeverity severity);

        /// @brief Begins error recovery mode (suppresses logging)
        void beginRecovery();

        /// @brief Ends error recovery mode
        void endRecovery();

        /// @brief Checks if any errors (not warnings) have been reported
        bool hasErrors() const;

        /// @brief Gets the list of recorded errors
        const std::vector<Error>& getErrors() const { return errors; }

    private:
        std::vector<Error> errors;   ///< List of errors
        bool inRecovery = false;     ///< Recovery mode flag
        mutable std::mutex mutex;    ///< Ensures thread-safe error reporting
    };

} // namespace error