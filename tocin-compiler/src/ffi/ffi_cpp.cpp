// File: src/ffi/ffi_cpp.cpp
#include "../ffi/ffi_cpp.h"
#include <stdexcept>
#include <iostream>

namespace ffi {

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
        registerFunction("print", [](const std::vector<FFIValue>& args) -> FFIValue {
            for (size_t i = 0; i < args.size(); ++i) {
                printFFIValue(args[i]);
                if (i < args.size() - 1) std::cout << " ";
            }
            std::cout << std::endl;
            return FFIValue(); // Null return
            });
    }

    FFIValue CppFFI::callCpp(const std::string& functionName, const std::vector<FFIValue>& args) {
        auto it = functions.find(functionName);
        if (it == functions.end()) {
            throw std::runtime_error("C++ function not found: " + functionName);
        }
        return it->second(args);
    }

    void CppFFI::registerFunction(const std::string& name, CppFunction func) {
        functions[name] = std::move(func);
    }

    FFIValue CppFFI::callJavaScript(const std::string& functionName, const std::vector<FFIValue>& args) {
        throw std::runtime_error("JavaScript calls not supported in C++ FFI");
    }

    FFIValue CppFFI::callPython(const std::string& moduleName, const std::string& functionName,
        const std::vector<FFIValue>& args) {
        throw std::runtime_error("Python calls not supported in C++ FFI");
    }

} // namespace ffi