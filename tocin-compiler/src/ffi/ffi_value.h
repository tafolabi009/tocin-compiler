// File: src/ffi/ffi_value.h
#pragma once

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <stdexcept>

namespace ffi {

    /// @brief Types supported by FFI values
    enum class FFIValueType { String, Integer, Double, Boolean, Null, Array, Object };

    /// @brief Represents a value passed to or from an FFI call
    class FFIValue {
    public:
        using ArrayType = std::vector<FFIValue>;
        using ObjectType = std::unordered_map<std::string, FFIValue>;
        using ValueType = std::variant<int64_t, double, bool, std::string, std::nullptr_t, ArrayType, ObjectType>;

        /// @brief Constructs an uninitialized (null) FFI value
        FFIValue() : value(nullptr) {}

        /// @brief Constructs an FFI value from a string
        explicit FFIValue(const std::string& str) : value(str) {}
        /// @brief Constructs an FFI value from an integer
        explicit FFIValue(int64_t num) : value(num) {}
        /// @brief Constructs an FFI value from a double
        explicit FFIValue(double num) : value(num) {}
        /// @brief Constructs an FFI value from a boolean
        explicit FFIValue(bool b) : value(b) {}
        /// @brief Constructs an FFI value from an array
        explicit FFIValue(const ArrayType& arr) : value(arr) {}
        /// @brief Constructs an FFI value from an object
        explicit FFIValue(const ObjectType& obj) : value(obj) {}
        /// @brief Constructs an FFI value from a variant
        explicit FFIValue(ValueType val) : value(std::move(val)) {}

        /// @brief Gets the type of the stored value
        FFIValueType getType() const {
            return std::visit([](auto&& arg) -> FFIValueType {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>) return FFIValueType::String;
                else if constexpr (std::is_same_v<T, int64_t>) return FFIValueType::Integer;
                else if constexpr (std::is_same_v<T, double>) return FFIValueType::Double;
                else if constexpr (std::is_same_v<T, bool>) return FFIValueType::Boolean;
                else if constexpr (std::is_same_v<T, std::nullptr_t>) return FFIValueType::Null;
                else if constexpr (std::is_same_v<T, ArrayType>) return FFIValueType::Array;
                else if constexpr (std::is_same_v<T, ObjectType>) return FFIValueType::Object;
                return FFIValueType::Null; // Shouldn't happen
                }, value);
        }

        /// @brief Gets the string value, throws if not a string
        std::string getString() const {
            if (getType() != FFIValueType::String) throw std::runtime_error("FFIValue is not a string");
            return std::get<std::string>(value);
        }

        /// @brief Gets the integer value, throws if not an integer
        int64_t getInteger() const {
            if (getType() != FFIValueType::Integer) throw std::runtime_error("FFIValue is not an integer");
            return std::get<int64_t>(value);
        }

        /// @brief Gets the double value, throws if not a double
        double getDouble() const {
            if (getType() != FFIValueType::Double) throw std::runtime_error("FFIValue is not a double");
            return std::get<double>(value);
        }

        /// @brief Gets the boolean value, throws if not a boolean
        bool getBoolean() const {
            if (getType() != FFIValueType::Boolean) throw std::runtime_error("FFIValue is not a boolean");
            return std::get<bool>(value);
        }

        /// @brief Gets the array value, throws if not an array
        ArrayType getArray() const {
            if (getType() != FFIValueType::Array) throw std::runtime_error("FFIValue is not an array");
            return std::get<ArrayType>(value);
        }

        /// @brief Gets the object value, throws if not an object
        ObjectType getObject() const {
            if (getType() != FFIValueType::Object) throw std::runtime_error("FFIValue is not an object");
            return std::get<ObjectType>(value);
        }

        /// @brief Checks if the value is null
        bool isNull() const { return getType() == FFIValueType::Null; }

        /// @brief Gets the raw variant value
        const ValueType& getValue() const { return value; }

    private:
        ValueType value;
    };

} // namespace ffi

#endif