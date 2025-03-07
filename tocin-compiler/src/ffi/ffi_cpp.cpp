#include "ffi_cpp.h"
#include <stdexcept>
#include <iostream>
#include <variant>

namespace ffi {

    // Helper functor to print an FFIValue
    struct FFIValuePrinter {
        void operator()(int64_t val) const { std::cout << val; }
        void operator()(double val) const { std::cout << val; }
        void operator()(bool val) const { std::cout << (val ? "true" : "false"); }
        void operator()(const std::string& val) const { std::cout << val; }
        void operator()(std::nullptr_t) const { std::cout << "null"; }
        void operator()(const FFIValue::ArrayType& arr) const {
            std::cout << "[";
            bool first = true;
            for (const auto& item : arr) {
                if (!first) std::cout << ", ";
                std::visit(FFIValuePrinter(), item.getValue());
                first = false;
            }
            std::cout << "]";
        }
        void operator()(const FFIValue::ObjectType& obj) const {
            std::cout << "{";
            bool first = true;
            for (const auto& [key, value] : obj) {
                if (!first) std::cout << ", ";
                std::cout << key << ": ";
                std::visit(FFIValuePrinter(), value.getValue());
                first = false;
            }
            std::cout << "}";
        }
    };

    static void printFFIValue(const FFIValue& value) {
        std::visit(FFIValuePrinter(), value.getValue());
    }

    CppFFI::CppFFI() {
        // Register a built-in "print" function
        registerFunction("print", [](const std::vector<FFIValue>& args) -> FFIValue {
            for (const auto& arg : args) {
                printFFIValue(arg);
                std::cout << " ";
            }
            std::cout << std::endl;
            return FFIValue();
            });
    }

    FFIValue CppFFI::callCpp(const std::string& functionName,
        const std::vector<FFIValue>& args) {
        try {
            auto it = functions.find(functionName);
            if (it == functions.end()) {
                throw std::runtime_error("C++ function not found: " + functionName);
            }
            return it->second(args);
        }
        catch (const std::exception& e) {
            throw std::runtime_error("C++ FFI error: " + std::string(e.what()));
        }
    }

    void CppFFI::registerFunction(const std::string& name, CppFunction func) {
        functions[name] = std::move(func);
    }

    // Not supported in C++ FFI
    FFIValue CppFFI::callJavaScript(const std::string& functionName,
        const std::vector<FFIValue>& args) {
        throw std::runtime_error("JavaScript calls not supported in C++ FFI");
    }

    FFIValue CppFFI::callPython(const std::string& moduleName,
        const std::string& functionName,
        const std::vector<FFIValue>& args) {
        throw std::runtime_error("Python calls not supported in C++ FFI");
    }

} // namespace ffi
