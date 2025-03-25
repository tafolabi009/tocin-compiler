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

    class CompilationContext {
    public:
        explicit CompilationContext(const std::string& filename);
        void initialize();
        ~CompilationContext();

        std::string filename;
        std::shared_ptr<error::ErrorHandler> errorHandler;
        std::shared_ptr<ffi::JavaScriptFFI> javascriptFFI;
        std::shared_ptr<ffi::PythonFFI> pythonFFI;
        std::shared_ptr<ffi::CppFFI> cppFFI;
        std::unique_ptr<llvm::LLVMContext> llvmContext;
        std::unique_ptr<llvm::Module> module;
        bool hasErrors = false;

    private:
        bool pythonInitialized = false;
        void registerBuiltInFunctions();
    };

} // namespace compiler