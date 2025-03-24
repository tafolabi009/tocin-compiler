// File: src/compiler/compilation_context.cpp
#include "compilation_context.h"
#include "../ffi/ffi_value.h"        // For ffi::FFIValue
#include "../error/error_handler.h"  // For error::CompilerError
#ifdef _WIN32
#define MS_WINDOWS
#endif
#include <Python.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "../ffi/ffi_javascript.h"
#include "../ffi/ffi_python.h"
#include "../ffi/ffi_cpp.h"

CompilationContext::CompilationContext(const std::string& filename)
    : filename(filename), optimizationsEnabled(false), debugInfoEnabled(false), optimizationLevel(0) {
    initialize();
}

void CompilationContext::initialize() {
    try {
        errorHandler = std::make_shared<error::ErrorHandler>();
        javascriptFFI = std::make_shared<ffi::JavaScriptFFI>();
        pythonFFI = std::make_shared<ffi::PythonFFI>();
        cppFFI = std::make_shared<ffi::CppFFI>();

        auto cpp = std::static_pointer_cast<ffi::CppFFI>(cppFFI);
        cpp->registerFunction("sizeof", [](const std::vector<ffi::FFIValue>& args) -> ffi::FFIValue {
            if (args.empty()) return ffi::FFIValue(0LL);

            const ffi::FFIValue& arg = args[0];
            if (arg.getType() == ffi::FFIValueType::String) {
                std::string typeName = arg.getString();
                if (typeName == "int") return ffi::FFIValue(static_cast<int64_t>(sizeof(int)));
                if (typeName == "float") return ffi::FFIValue(static_cast<int64_t>(sizeof(float)));
                if (typeName == "double") return ffi::FFIValue(static_cast<int64_t>(sizeof(double)));
                return ffi::FFIValue(0LL);
            }
            else if (arg.getType() == ffi::FFIValueType::Integer) {
                int64_t typeId = arg.getInteger();
                switch (typeId) {
                case 0: return ffi::FFIValue(static_cast<int64_t>(sizeof(char)));
                case 1: return ffi::FFIValue(static_cast<int64_t>(sizeof(int)));
                default: return ffi::FFIValue(0LL);
                }
            }
            return ffi::FFIValue(static_cast<int64_t>(sizeof(int64_t)));
            });
    }
    catch (const std::exception& e) {
        if (errorHandler) {
            // Create a CompilerError object and then pass it to reportError
            error::CompilerError err(
                "Initialization error: " + std::string(e.what()),
                filename,
                0,
                0,
				error::ErrorSeverity::FATAL 
            );
            errorHandler->reportError(err);
        }
    } // Added missing closing brace for try-catch block
}


void CompilationContext::cleanup() {
    if (errorHandler) {
        errorHandler->clear();
    }

    // Reset FFI interfaces
    javascriptFFI.reset();
    pythonFFI.reset();
    cppFFI.reset();
}

bool CompilationContext::hasErrors() const {
    return errorHandler && errorHandler->hasErrors();
}

void CompilationContext::enableOptimizations(int level) {
    if (level < 0) {
        level = 0; // Ensure optimization level is not negative
    }
    else if (level > 3) {
        level = 3; // Cap optimization level at 3 (assuming this is the maximum)
    }

    optimizationsEnabled = (level > 0);
    optimizationLevel = level;
}

void CompilationContext::enableDebugInfo() {
    debugInfoEnabled = true;
}