// File: src/ffi/ffi_cpp.h
#pragma once

#include "ffi_interface.h"
#include <unordered_map>
#include <functional>

namespace ffi {

    /// @brief C++ implementation of the FFI interface
    class CppFFI : public FFIInterface {
    public:
        using CppFunction = std::function<FFIValue(const std::vector<FFIValue>&)>;

        /// @brief Constructs a C++ FFI instance with default functions
        CppFFI();
        ~CppFFI() override = default;

        /// @brief Calls a JavaScript function (unsupported in C++ FFI)
        FFIValue callJavaScript(const std::string& functionName, const std::vector<FFIValue>& args) override;

        /// @brief Calls a Python function (unsupported in C++ FFI)
        FFIValue callPython(const std::string& moduleName, const std::string& functionName,
            const std::vector<FFIValue>& args) override;

        /// @brief Calls a registered C++ function
        FFIValue callCpp(const std::string& functionName, const std::vector<FFIValue>& args) override;

        /// @brief Registers a C++ function for external calls
        /// @param name Name of the function
        /// @param func Function implementation
        void registerFunction(const std::string& name, CppFunction func);

    private:
        std::unordered_map<std::string, CppFunction> functions; ///< Registered C++ functions
    };

} // namespace ffi