#pragma once

#include "ffi_interface.h"
#include <v8.h>
#include <future>

namespace ffi {

    class JavaScriptFFI : public FFIInterface {
    public:
        JavaScriptFFI();
        ~JavaScriptFFI();

        FFIValue callJavaScript(const std::string& functionName,
            const std::vector<FFIValue>& args) override;

        // (Optional) You can override callJavaScriptAsync if desired.

        FFIValue callPython(const std::string& moduleName,
            const std::string& functionName,
            const std::vector<FFIValue>& args) override;

        FFIValue callCpp(const std::string& functionName,
            const std::vector<FFIValue>& args) override;

    private:
        v8::Isolate* isolate;
        v8::Global<v8::Context> context;

        v8::Local<v8::Value> convertToV8Value(const FFIValue& value);
        FFIValue convertFromV8Value(v8::Local<v8::Value> value);
    };

} // namespace ffi
