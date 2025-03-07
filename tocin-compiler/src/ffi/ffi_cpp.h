#pragma once

#include "ffi_interface.h"
#include <map>
#include <functional>

namespace ffi {

    class CppFFI : public FFIInterface {
    public:
        using CppFunction = std::function<FFIValue(const std::vector<FFIValue>&)>;

        CppFFI();
        ~CppFFI() = default;

        FFIValue callJavaScript(const std::string& functionName,
            const std::vector<FFIValue>& args) override;

        FFIValue callPython(const std::string& moduleName,
            const std::string& functionName,
            const std::vector<FFIValue>& args) override;

        FFIValue callCpp(const std::string& functionName,
            const std::vector<FFIValue>& args) override;

        // Register a C++ function for external calls
        void registerFunction(const std::string& name, CppFunction func);

    private:
        std::map<std::string, CppFunction> functions;
    };

} // namespace ffi
