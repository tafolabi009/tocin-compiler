#pragma once

#include <memory>
#include <string>
#include "../error/error_handler.h"
#include "../ffi/ffi_interface.h"

class CompilationContext {
public:
    CompilationContext(const std::string& filename);
    ~CompilationContext() = default;

    // Error handling
    std::shared_ptr<error::ErrorHandler> errorHandler;

    // FFI interfaces (now using the enhanced versions)
    std::shared_ptr<ffi::FFIInterface> javascriptFFI;
    std::shared_ptr<ffi::FFIInterface> pythonFFI;
    std::shared_ptr<ffi::FFIInterface> cppFFI;

    // Source file information
    std::string filename;
    std::string sourceCode;

    // Compilation flags
    bool optimizationsEnabled;
    bool debugInfoEnabled;
    int optimizationLevel;

    // Methods
    void initialize();
    void cleanup();
    bool hasErrors() const;
    void enableOptimizations(int level = 1);
    void enableDebugInfo();
};
