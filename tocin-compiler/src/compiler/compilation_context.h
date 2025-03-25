// File: src/compiler/compilation_context.h
#pragma once

#include <string>
#include <memory>
#include "../error/error_handler.h"
#include "../ffi/ffi_interface.h"
#include "../ffi/ffi_javascript.h"
#include "../ffi/ffi_python.h"
#include "../ffi/ffi_cpp.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace compiler {

    /// @brief Manages the compilation process and shared resources
    class CompilationContext {
    public:
        /// @brief Constructs a compilation context with a source filename
        explicit CompilationContext(const std::string& filename);

        /// @brief Initializes FFI interfaces and LLVM context
        void initialize();

        /// @brief Cleans up resources
        ~CompilationContext();

        std::string filename;                          ///< Source file being compiled
        std::shared_ptr<error::ErrorHandler> errorHandler; ///< Error reporting system
        std::shared_ptr<ffi::JavaScriptFFI> javascriptFFI; ///< JavaScript FFI interface
        std::shared_ptr<ffi::PythonFFI> pythonFFI;         ///< Python FFI interface
        std::shared_ptr<ffi::CppFFI> cppFFI;               ///< C++ FFI interface
        std::unique_ptr<llvm::LLVMContext> llvmContext;    ///< LLVM context for IR generation
        std::unique_ptr<llvm::Module> module;              ///< LLVM module for IR
        bool hasErrors = false;                            ///< Tracks if errors occurred

    private:
        bool pythonInitialized = false;                    ///< Tracks Python interpreter state

        /// @brief Registers built-in C++ functions
        void registerBuiltInFunctions();
    };

} // namespace compiler