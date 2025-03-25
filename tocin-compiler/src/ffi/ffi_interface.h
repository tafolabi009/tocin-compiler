// File: src/ffi/ffi_interface.h
#pragma once

#include <string>
#include <vector>
#include <future>
#include "ffi_value.h"

namespace ffi {

    /// @brief Interface for Foreign Function Interface (FFI) implementations
    class FFIInterface {
    public:
        virtual ~FFIInterface() = default;

        /// @brief Synchronously calls a JavaScript function
        /// @param functionName Name of the JavaScript function
        /// @param args Arguments to pass to the function
        /// @return Result of the function call
        virtual FFIValue callJavaScript(const std::string& functionName, const std::vector<FFIValue>& args) = 0;

        /// @brief Asynchronously calls a JavaScript function
        /// @param functionName Name of the JavaScript function
        /// @param args Arguments to pass to the function
        /// @return Future containing the result of the function call
        virtual std::future<FFIValue> callJavaScriptAsync(const std::string& functionName,
            const std::vector<FFIValue>& args) {
            return std::async(std::launch::async, [this, functionName, args]() {
                return callJavaScript(functionName, args);
                });
        }

        /// @brief Calls a Python function from a module
        /// @param moduleName Name of the Python module
        /// @param functionName Name of the function within the module
        /// @param args Arguments to pass to the function
        /// @return Result of the function call
        virtual FFIValue callPython(const std::string& moduleName,
            const std::string& functionName,
            const std::vector<FFIValue>& args) = 0;

        /// @brief Calls a registered C++ function
        /// @param functionName Name of the C++ function
        /// @param args Arguments to pass to the function
        /// @return Result of the function call
        virtual FFIValue callCpp(const std::string& functionName, const std::vector<FFIValue>& args) = 0;
    };

} // namespace ffi