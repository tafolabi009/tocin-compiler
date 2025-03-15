// File: compilation_context.cpp
#include "compilation_context.h"
#ifdef _WIN32
#define MS_WINDOWS
#endif
#include <Python.h>

// Handle platform-specific includes properly
#ifdef _WIN32
// Windows-specific headers that might be needed instead of unistd.h
#include <windows.h>
#include <io.h>
// Remove the unistd.h completely for Windows - no conditional inclusion needed
#else
// Unix/Linux/macOS systems can use unistd.h
#include <unistd.h>
#endif

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
    try {
        // Initialize error handler
        errorHandler = std::make_shared<error::ErrorHandler>();

        // Initialize enhanced FFI interfaces
        javascriptFFI = std::make_shared<ffi::JavaScriptFFI>();
        pythonFFI = std::make_shared<ffi::PythonFFI>();
        cppFFI = std::make_shared<ffi::CppFFI>();

        // Register a default C++ function
        auto cpp = std::static_pointer_cast<ffi::CppFFI>(cppFFI);
        cpp->registerFunction("sizeof", [](const std::vector<ffi::FFIValue>& args) -> ffi::FFIValue {
            if (args.empty()) {
                return ffi::FFIValue(0LL);
            }
            
            // Check the argument type and handle accordingly
            const ffi::FFIValue& arg = args[0];
            if (arg.getType() == ffi::FFIValueType::String) {
                // Get size based on type string
                std::string typeName = arg.getString();
                if (typeName == "int") return ffi::FFIValue(static_cast<int64_t>(sizeof(int)));
                if (typeName == "float") return ffi::FFIValue(static_cast<int64_t>(sizeof(float)));
                if (typeName == "double") return ffi::FFIValue(static_cast<int64_t>(sizeof(double)));
                if (typeName == "int64_t") return ffi::FFIValue(static_cast<int64_t>(sizeof(int64_t)));
                if (typeName == "char") return ffi::FFIValue(static_cast<int64_t>(sizeof(char)));
                if (typeName == "bool") return ffi::FFIValue(static_cast<int64_t>(sizeof(bool)));
                if (typeName == "size_t") return ffi::FFIValue(static_cast<int64_t>(sizeof(size_t)));
                if (typeName == "void*") return ffi::FFIValue(static_cast<int64_t>(sizeof(void*)));
                // Default case
                return ffi::FFIValue(0LL);
            } else if (arg.getType() == ffi::FFIValueType::Integer) {
                // Handle integer type argument (could be an enum value representing types)
                int64_t typeId = arg.getInteger();
                switch (typeId) {
                    case 0: return ffi::FFIValue(static_cast<int64_t>(sizeof(char)));
                    case 1: return ffi::FFIValue(static_cast<int64_t>(sizeof(int)));
                    case 2: return ffi::FFIValue(static_cast<int64_t>(sizeof(float)));
                    case 3: return ffi::FFIValue(static_cast<int64_t>(sizeof(double)));
                    case 4: return ffi::FFIValue(static_cast<int64_t>(sizeof(int64_t)));
                    default: return ffi::FFIValue(0LL);
                }
            }
            
            // Default to sizeof(int64_t) for other cases
            return ffi::FFIValue(static_cast<int64_t>(sizeof(int64_t)));
        });
    } catch (const std::exception& e) {
        // Log initialization error
        if (errorHandler) {
            errorHandler->logError("Initialization error: " + std::string(e.what()));
        }
    }
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
    } else if (level > 3) {
        level = 3; // Cap optimization level at 3 (assuming this is the maximum)
    }
    
    optimizationsEnabled = (level > 0);
    optimizationLevel = level;
}

void CompilationContext::enableDebugInfo() {
    debugInfoEnabled = true;
}
