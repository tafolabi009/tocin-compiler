// File: src/ffi/ffi_javascript.cpp
#include "ffi_javascript.h"
#include <stdexcept>
#include <iostream>

namespace {

    void sanitizeFFIArgs(const std::vector<ffi::FFIValue>& args, size_t maxArraySize = 1000, size_t maxStringLength = 10000) {
        for (const auto& arg : args) {
            if (arg.getType() == ffi::FFIValueType::Array) {
                auto arr = arg.getArray();
                if (arr.size() > maxArraySize) {
                    throw std::runtime_error("FFI input array exceeds maximum allowed size (" + std::to_string(maxArraySize) + ")");
                }
            }
            else if (arg.getType() == ffi::FFIValueType::String) {
                auto str = arg.getString();
                if (str.length() > maxStringLength) {
                    throw std::runtime_error("FFI input string exceeds maximum allowed length (" + std::to_string(maxStringLength) + ")");
                }
            }
        }
    }

} // anonymous namespace

namespace ffi {

    JavaScriptFFI::JavaScriptFFI() {
        static bool v8Initialized = false;
        if (!v8Initialized) {
            v8::V8::InitializeICUDefaultLocation(nullptr);
            v8::V8::InitializeExternalStartupData(nullptr);
            v8::V8::Initialize();
            v8Initialized = true;
        }

        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        isolate = v8::Isolate::New(create_params);

        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);

        auto ctx = v8::Context::New(isolate);
        context.Reset(isolate, ctx);
    }

    JavaScriptFFI::~JavaScriptFFI() {
        context.Reset();
        isolate->Dispose();
        // Note: V8::Dispose() is not called here to avoid conflicts with multiple instances
        delete isolate->GetArrayBufferAllocator();
    }

    FFIValue JavaScriptFFI::callJavaScript(const std::string& functionName, const std::vector<FFIValue>& args) {
        sanitizeFFIArgs(args);

        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        auto localContext = v8::Local<v8::Context>::New(isolate, context);
        v8::Context::Scope context_scope(localContext);

        try {
            auto global = localContext->Global();
            v8::Local<v8::Value> funcValue;
            if (!global->Get(localContext, v8::String::NewFromUtf8(isolate, functionName.c_str()).ToLocalChecked())
                .ToLocal(&funcValue) || !funcValue->IsFunction()) {
                throw std::runtime_error("JavaScript function not found: " + functionName);
            }
            auto func = v8::Local<v8::Function>::Cast(funcValue);

            std::vector<v8::Local<v8::Value>> v8Args(args.size());
            for (size_t i = 0; i < args.size(); ++i) {
                v8Args[i] = convertToV8Value(args[i]);
            }

            v8::Local<v8::Value> result;
            if (!func->Call(localContext, global, static_cast<int>(v8Args.size()), v8Args.data()).ToLocal(&result)) {
                throw std::runtime_error("JavaScript function call failed: " + functionName);
            }
            return convertFromV8Value(result);
        }
        catch (const std::exception& e) {
            throw std::runtime_error("JavaScript FFI error: " + std::string(e.what()));
        }
    }

    v8::Local<v8::Value> JavaScriptFFI::convertToV8Value(const FFIValue& value) {
        struct Visitor {
            v8::Isolate* isolate;
            v8::Local<v8::Context> context;
            Visitor(v8::Isolate* iso, v8::Local<v8::Context> ctx) : isolate(iso), context(ctx) {}

            v8::Local<v8::Value> operator()(int64_t val) const { return v8::Number::New(isolate, static_cast<double>(val)); }
            v8::Local<v8::Value> operator()(double val) const { return v8::Number::New(isolate, val); }
            v8::Local<v8::Value> operator()(bool val) const { return v8::Boolean::New(isolate, val); }
            v8::Local<v8::Value> operator()(const std::string& val) const {
                return v8::String::NewFromUtf8(isolate, val.c_str()).ToLocalChecked();
            }
            v8::Local<v8::Value> operator()(std::nullptr_t) const { return v8::Null(isolate); }
            v8::Local<v8::Value> operator()(const FFIValue::ArrayType& arr) const {
                v8::Local<v8::Array> jsArr = v8::Array::New(isolate, static_cast<int>(arr.size()));
                for (size_t i = 0; i < arr.size(); ++i) {
                    jsArr->Set(context, static_cast<uint32_t>(i), std::visit(*this, arr[i].getValue())).FromMaybe(false);
                }
                return jsArr;
            }
            v8::Local<v8::Value> operator()(const FFIValue::ObjectType& obj) const {
                v8::Local<v8::Object> jsObj = v8::Object::New(isolate);
                for (const auto& [key, val] : obj) {
                    jsObj->Set(context, v8::String::NewFromUtf8(isolate, key.c_str()).ToLocalChecked(),
                        std::visit(*this, val.getValue())).FromMaybe(false);
                }
                return jsObj;
            }
        };

        v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
        return std::visit(Visitor(isolate, ctx), value.getValue());
    }

    FFIValue JavaScriptFFI::convertFromV8Value(v8::Local<v8::Value> value) {
        v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
        if (value->IsNull() || value->IsUndefined()) {
            return FFIValue();
        }

        if (value->IsNumber()) {
            double num = value->NumberValue(ctx).ToChecked();
            if (std::floor(num) == num && num <= INT64_MAX && num >= INT64_MIN) {
                return FFIValue(static_cast<int64_t>(num));
            }
            return FFIValue(num);
        }
        if (value->IsBoolean()) {
            return FFIValue(value->BooleanValue(isolate));
        }
        if (value->IsString()) {
            v8::String::Utf8Value str(isolate, value);
            return FFIValue(std::string(*str));
        }
        if (value->IsArray()) {
            v8::Local<v8::Array> jsArr = v8::Local<v8::Array>::Cast(value);
            FFIValue::ArrayType arr(jsArr->Length());
            for (uint32_t i = 0; i < jsArr->Length(); ++i) {
                v8::Local<v8::Value> elem;
                if (jsArr->Get(ctx, i).ToLocal(&elem)) {
                    arr[i] = convertFromV8Value(elem);
                }
            }
            return FFIValue(arr);
        }
        if (value->IsObject()) {
            v8::Local<v8::Object> jsObj = value->ToObject(ctx).ToLocalChecked();
            FFIValue::ObjectType obj;
            v8::Local<v8::Array> propertyNames = jsObj->GetOwnPropertyNames(ctx).ToLocalChecked();
            for (uint32_t i = 0; i < propertyNames->Length(); ++i) {
                v8::Local<v8::Value> key;
                if (!propertyNames->Get(ctx, i).ToLocal(&key)) continue;
                v8::String::Utf8Value utf8Key(isolate, key);
                std::string keyStr(*utf8Key);
                v8::Local<v8::Value> val;
                if (jsObj->Get(ctx, key).ToLocal(&val)) {
                    obj[keyStr] = convertFromV8Value(val);
                }
            }
            return FFIValue(obj);
        }
        throw std::runtime_error("Unsupported JavaScript value type");
    }

    FFIValue JavaScriptFFI::callPython(const std::string& moduleName, const std::string& functionName,
        const std::vector<FFIValue>& args) {
        throw std::runtime_error("Python calls not supported in JavaScript FFI");
    }

    FFIValue JavaScriptFFI::callCpp(const std::string& functionName, const std::vector<FFIValue>& args) {
        throw std::runtime_error("C++ calls not supported in JavaScript FFI");
    }

} // namespace ffi