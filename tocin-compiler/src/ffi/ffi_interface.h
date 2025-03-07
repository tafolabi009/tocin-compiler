#pragma once

#include <string>
#include <memory>
#include <variant>
#include <vector>
#include <unordered_map>
#include <future>
#include "../ast/ast.h"

namespace ffi {

    class FFIValue {
    public:
        using ArrayType = std::vector<FFIValue>;
        using ObjectType = std::unordered_map<std::string, FFIValue>;
        using ValueType = std::variant<
            int64_t,
            double,
            bool,
            std::string,
            std::nullptr_t,
            ArrayType,
            ObjectType
        >;

        FFIValue() : value(nullptr) {}
        FFIValue(ValueType val) : value(std::move(val)) {}

        template<typename T>
        T as() const {
            return std::get<T>(value);
        }

        bool isNull() const { return std::holds_alternative<std::nullptr_t>(value); }

        const ValueType& getValue() const { return value; }

    private:
        ValueType value;
    };

    class FFIInterface {
    public:
        virtual ~FFIInterface() = default;

        // Synchronous JavaScript call
        virtual FFIValue callJavaScript(const std::string& functionName,
            const std::vector<FFIValue>& args) = 0;

        // Default asynchronous call (can be overridden for a more sophisticated event loop)
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
