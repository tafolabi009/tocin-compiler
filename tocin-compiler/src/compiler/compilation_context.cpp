#pragma once
// File: compilation_context.cpp
#include "compilation_context.h"
#include "../ffi/ffi_javascript.h"
#include "../ffi/ffi_python.h"
#include "../ffi/ffi_cpp.h"

CompilationContext::CompilationContext(const std::string& filename)
    : filename(filename),
    optimizationsEnabled(false),
    debugInfoEnabled(false),
    optimizationLevel(0) {
    initialize();
}

void CompilationContext::initialize() {
    // Initialize error handler
    errorHandler = std::make_shared<error::ErrorHandler>();

    // Initialize enhanced FFI interfaces
    javascriptFFI = std::make_shared<ffi::JavaScriptFFI>();
    pythonFFI = std::make_shared<ffi::PythonFFI>();
    cppFFI = std::make_shared<ffi::CppFFI>();

    // Register a default C++ function (for demonstration)
    auto cpp = std::static_pointer_cast<ffi::CppFFI>(cppFFI);
    cpp->registerFunction("sizeof", [](const std::vector<ffi::FFIValue>& args) -> ffi::FFIValue {
        if (args.empty()) return ffi::FFIValue(0LL);
        return ffi::FFIValue(static_cast<int64_t>(sizeof(int64_t)));
        });
}

void CompilationContext::cleanup() {
    errorHandler->clear();
    // Any additional cleanup for FFI interfaces would be performed in their destructors.
}

bool CompilationContext::hasErrors() const {
    return errorHandler->hasErrors();
}

void CompilationContext::enableOptimizations(int level) {
    optimizationsEnabled = true;
    optimizationLevel = level;
}

void CompilationContext::enableDebugInfo() {
    debugInfoEnabled = true;
}
