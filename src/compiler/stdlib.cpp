#include "stdlib.h"
#include <cmath>
#include <iostream>
#include <ostream>

namespace tocin {
namespace compiler {

    void StdLib::registerFunctions(ffi::CppFFI& ffi) {
        ffi.registerFunction("print", [](const std::vector<ffi::FFIValue>& args) -> ffi::FFIValue {
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) std::cout << " ";
                switch (args[i].getType()) {
                case ffi::FFIValue::INTEGER: std::cout << args[i].asInt64(); break;
                case ffi::FFIValue::FLOAT: std::cout << args[i].asDouble(); break;
                case ffi::FFIValue::BOOLEAN: std::cout << (args[i].asBoolean() ? "true" : "false"); break;
                case ffi::FFIValue::STRING: std::cout << args[i].asString(); break;
                default: std::cout << "unknown";
                }
            }
            std::cout << std::endl;
            return ffi::FFIValue();
            });

        ffi.registerFunction("str_length", [](const std::vector<ffi::FFIValue>& args) -> ffi::FFIValue {
            if (args.size() != 1 || args[0].getType() != ffi::FFIValue::STRING) {
                throw std::runtime_error("str_length expects one string");
            }
            return ffi::FFIValue(static_cast<int64_t>(args[0].asString().size()));
            });

        ffi.registerFunction("str_concat", [](const std::vector<ffi::FFIValue>& args) -> ffi::FFIValue {
            if (args.size() != 2 || args[0].getType() != ffi::FFIValue::STRING ||
                args[1].getType() != ffi::FFIValue::STRING) {
                throw std::runtime_error("str_concat expects two strings");
            }
            return ffi::FFIValue(args[0].asString() + args[1].asString());
            });

        ffi.registerFunction("str_slice", [](const std::vector<ffi::FFIValue>& args) -> ffi::FFIValue {
            if (args.size() != 3 || args[0].getType() != ffi::FFIValue::STRING ||
                args[1].getType() != ffi::FFIValue::INTEGER || args[2].getType() != ffi::FFIValue::INTEGER) {
                throw std::runtime_error("str_slice expects string, start, end");
            }
            auto str = args[0].asString();
            int64_t start = args[1].asInt64();
            int64_t end = args[2].asInt64();
            if (start < 0 || end > static_cast<int64_t>(str.size()) || start > end) {
                return ffi::FFIValue("");
            }
            return ffi::FFIValue(str.substr(start, end - start));
            });

        ffi.registerFunction("math_sqrt", [](const std::vector<ffi::FFIValue>& args) -> ffi::FFIValue {
            if (args.size() != 1 || args[0].getType() != ffi::FFIValue::FLOAT) {
                throw std::runtime_error("sqrt expects one float");
            }
            return ffi::FFIValue(std::sqrt(args[0].asDouble()));
            });
    }

} // namespace compiler
} // namespace tocin