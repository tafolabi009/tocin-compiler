// File: src/ffi/ffi_javascript.h
#pragma once

#include "ffi_interface.h"
#include <v8.h>
#include <string>
#include <vector>

namespace ffi {

    /// @brief JavaScript implementation of the FFI interface using V8
    class JavaScriptFFI : public FFIInterface {
    public:
        /// @brief Constructs a JavaScript FFI instance, initializing V8
        JavaScriptFFI();

        /// @brief Cleans up V8 resources
        ~JavaScriptFFI() override;

        /// @brief Calls a JavaScript function in the global context
        FFIValue callJavaScript(const std::string& functionName, const std::vector<FFIValue>& args) override;

        /// @brief Calls a Python function (unsupported in JavaScript FFI)
        FFIValue callPython(const std::string& moduleName, const std::string& functionName,
            const std::vector<FFIValue>& args) override;

        /// @brief Calls a C++ function (unsupported in JavaScript FFI)
        FFIValue callCpp(const std::string& functionName, const std::vector<FFIValue>& args) override;

    private:
        v8::Isolate* isolate;                     ///< V8 isolate for execution
        v8::Persistent<v8::Context> context;      ///< Persistent V8 context

        /// @brief Converts an FFIValue to a V8 value
        v8::Local<v8::Value> convertToV8Value(const FFIValue& value);

        /// @brief Converts a V8 value to an FFIValue
        FFIValue convertFromV8Value(v8::Local<v8::Value> value);
    };

} // namespace ffi