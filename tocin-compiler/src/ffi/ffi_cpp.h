#ifndef FFI_CPP_H
#define FFI_CPP_H

#include "ffi_interface.h"
#include <functional>
#include <unordered_map>

namespace ffi {

    class CppFFI : public FFIInterface {
    public:
        using CppFunction = std::function<FFIValue(const std::vector<FFIValue>&)>;

        CppFFI() = default;
        ~CppFFI() override = default;

        void registerFunction(const std::string& name, CppFunction func);
        FFIValue call(const std::string& functionName, const std::vector<FFIValue>& args) override;
        bool hasFunction(const std::string& functionName) const override;

    private:
        std::unordered_map<std::string, CppFunction> functions;
    };

} // namespace ffi

#endif // FFI_CPP_H