// File: src/error/error_handler.h
#pragma once

#include <string>
#include <vector>
#include "../lexer/token.h"
#include <mutex>

namespace error {

    enum class ErrorSeverity {
        WARNING,
        ERROR,
        FATAL
    };

    struct Error {
        std::string message;
        std::string filename;
        int line;
        int column;
        ErrorSeverity severity;

        Error(const std::string& msg, const std::string& fname, int l, int c, ErrorSeverity sev)
            : message(msg), filename(fname), line(l), column(c), severity(sev) {}
    };

    class ErrorHandler {
    public:
        void reportError(const std::string& message, const Token& token, ErrorSeverity severity);
        void reportError(const std::string& message, const std::string& filename, int line, int column,
            ErrorSeverity severity);
        void beginRecovery();
        void endRecovery();
        bool hasErrors() const;
        const std::vector<Error>& getErrors() const { return errors; }

    private:
        std::vector<Error> errors;
        bool inRecovery = false;
        mutable std::mutex mutex;
    };

} // namespace error