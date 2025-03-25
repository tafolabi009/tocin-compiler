// src/ffi/ffi_value.h
#ifndef FFI_VALUE_H
#define FFI_VALUE_H

#include <string>

namespace ffi {
	class FFIValue {
	public:
		FFIValue() = default;
		explicit FFIValue(int64_t i) : integer(i), type(Type::INTEGER) {}
		explicit FFIValue(std::string s) : string(std::move(s)), type(Type::STRING) {}

		int64_t getInteger() const { return integer; }
		std::string getString() const { return string; }

		enum class Type { INTEGER, STRING } type;

	private:
		int64_t integer = 0;
		std::string string;
	};
}

#endif // FFI_VALUE_H