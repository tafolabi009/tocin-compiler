// File: src/compiler/compilation_context.cpp
#include "compilation_context.h"
#include <stdexcept>
#include <python3.12/Python.h>
#include <mutex>

namespace compiler {

    namespace {
        std::mutex pythonInitMutex;
    }

    CompilationContext::CompilationContext(const std::string& fname) : filename(fname) {
        initialize();
    }

    void CompilationContext::initialize() {
        try {
            errorHandler = std::make_shared<error::ErrorHandler>();
            {
                std::lock_guard<std::mutex> lock(pythonInitMutex);
                if (!Py_IsInitialized()) {
                    Py_Initialize();
                    if (!Py_IsInitialized()) {
                        errorHandler->reportError(
                            "Failed to initialize Python interpreter", filename, 0, 0, error::ErrorSeverity::FATAL
                        );
                        throw std::runtime_error("Python initialization failed");
                    }
                    pythonInitialized = true;
                }
            }
            javascriptFFI = std::make_shared<ffi::JavaScriptFFI>();
            pythonFFI = std::make_shared<ffi::PythonFFI>();
            cppFFI = std::make_shared<ffi::CppFFI>();
            registerBuiltInFunctions();
            llvmContext = std::make_unique<llvm::LLVMContext>();
            module = std::make_unique<llvm::Module>(filename, *llvmContext);
        }
        catch (const std::exception& e) {
            if (errorHandler) {
                errorHandler->reportError(
                    "Failed to initialize compilation context: " + std::string(e.what()),
                    filename, 0, 0, error::ErrorSeverity::FATAL
                );
            }
            throw;
        }
    }

    void CompilationContext::registerBuiltInFunctions() {
        cppFFI->registerFunction("sizeof", [this](const std::vector<ffi::FFIValue>& args) -> ffi::FFIValue {
            try {
                if (args.empty()) {
                    errorHandler->reportError(
                        "sizeof requires at least one argument", filename, 0, 0, error::ErrorSeverity::ERROR
                    );
                    return ffi::FFIValue(0LL);
                }
                const ffi::FFIValue& arg = args[0];
                if (arg.getType() == ffi::FFIValueType::String) {
                    std::string typeName = arg.getString();
                    if (typeName == "int") return ffi::FFIValue(static_cast<int64_t>(sizeof(int)));
                    if (typeName == "float") return ffi::FFIValue(static_cast<int64_t>(sizeof(float)));
                    if (typeName == "double") return ffi::FFIValue(static_cast<int64_t>(sizeof(double)));
                    errorHandler->reportError(
                        "Unsupported type for sizeof: " + typeName, filename, 0, 0, error::ErrorSeverity::ERROR
                    );
                }
                else {
                    errorHandler->reportError(
                        "sizeof expects a string argument", filename, 0, 0, error::ErrorSeverity::ERROR
                    );
                }
                return ffi::FFIValue(0LL);
            }
            catch (const std::exception& e) {
                errorHandler->reportError(
                    "sizeof error: " + std::string(e.what()), filename, 0, 0, error::ErrorSeverity::ERROR
                );
                return ffi::FFIValue(0LL);
            }
            });

        cppFFI->registerFunction("print", [this](const std::vector<ffi::FFIValue>& args) -> ffi::FFIValue {
            std::ostringstream oss;
            for (size_t i = 0; i < args.size(); ++i) {
                std::visit([&oss](auto&& arg) { oss << arg; }, args[i].getValue());
                if (i < args.size() - 1) oss << " ";
            }
            std::cout << oss.str() << std::endl;
            return ffi::FFIValue();
            });
    }

    CompilationContext::~CompilationContext() {
        if (pythonInitialized && Py_IsInitialized()) {
            PyGILState_STATE gstate = PyGILState_Ensure();
            Py_Finalize();
            PyGILState_Release(gstate);
        }
    }

} // namespace compiler