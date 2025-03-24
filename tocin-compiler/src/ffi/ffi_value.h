// File: src/ffi/ffi_value.h
#ifndef FFI_VALUE_H
#define FFI_VALUE_H

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

namespace ffi {
    enum class FFIValueType { String, Integer, Double, Boolean, Null, Array, Object };

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

        // Default constructor
        FFIValue() : value(nullptr) {}

        // Legacy constructors from original ffi_value.h
        FFIValue(const std::string& str) : value(str) {}
        FFIValue(int64_t num) : value(num) {}

        // New constructor from ffi_interface.h
        FFIValue(ValueType val) : value(std::move(val)) {}

        // Legacy accessors from original ffi_value.h
        FFIValueType getType() const {
            if (std::holds_alternative<std::string>(value)) return FFIValueType::String;
            if (std::holds_alternative<int64_t>(value)) return FFIValueType::Integer;
            if (std::holds_alternative<double>(value)) return FFIValueType::Double;
            if (std::holds_alternative<bool>(value)) return FFIValueType::Boolean;
            if (std::holds_alternative<std::nullptr_t>(value)) return FFIValueType::Null;
            if (std::holds_alternative<ArrayType>(value)) return FFIValueType::Array;
            if (std::holds_alternative<ObjectType>(value)) return FFIValueType::Object;
            return FFIValueType::Null; // Default
        }

        std::string getString() const {
            return std::holds_alternative<std::string>(value) ?
                std::get<std::string>(value) : "";
        }

        int64_t getInteger() const {
            return std::holds_alternative<int64_t>(value) ?
                std::get<int64_t>(value) : 0;
        }

        // New accessors from ffi_interface.h
        template<typename T>
        T as() const {
            return std::get<T>(value);
        }

        bool isNull() const { return std::holds_alternative<std::nullptr_t>(value); }
        const ValueType& getValue() const { return value; }

    private:
        ValueType value;
    };
}

#endif