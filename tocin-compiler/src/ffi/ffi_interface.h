// File: src/ffi/ffi_interface.h
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <future>
#include "ffi_value.h"  // Include the unified FFIValue class
#include "../ast/ast.h"

namespace ffi {
    // FFIValue is now defined in ffi_value.h

    class FFIInterface {
    public:
        virtual ~FFIInterface() = default;

        // Synchronous JavaScript call
        virtual FFIValue callJavaScript(const std::string& functionName,
            const std::vector<FFIValue>& args) = 0;

        // Default asynchronous call
        virtual std::future<FFIValue> callJavaScriptAsync(const std::string& functionName,
            const std::vector<FFIValue>& args) {
            return std::async(std::launch::async, [this, functionName, args]() {
                return callJavaScript(functionName, args);
                });
        }

        // Python integration
        virtual FFIValue callPython(const std::string& moduleName,
            const std::string& functionName,
            const std::vector<FFIValue>& args) = 0;

        // C++ integration
        virtual FFIValue callCpp(const std::string& functionName,
            const std::vector<FFIValue>& args) = 0;
    };
} // namespace ffi