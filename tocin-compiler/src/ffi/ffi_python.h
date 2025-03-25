// File: src/ffi/ffi_python.h
#pragma once

#include "ffi_interface.h"
#include <python3.12/Python.h>
#include <vector>
#include <string>

namespace ffi {

    /// @brief Python implementation of the FFI interface
    class PythonFFI : public FFIInterface {
    public:
        /// @brief Constructs a Python FFI instance
        PythonFFI();

        /// @brief Cleans up Python resources
        ~PythonFFI() override;

        /// @brief Calls a JavaScript function (unsupported in Python FFI)
        FFIValue callJavaScript(const std::string& functionName, const std::vector<FFIValue>& args) override;

        /// @brief Calls a Python function from a module
        FFIValue callPython(const std::string& moduleName, const std::string& functionName,
            const std::vector<FFIValue>& args) override;

        /// @brief Calls a C++ function (unsupported in Python FFI)
        FFIValue callCpp(const std::string& functionName, const std::vector<FFIValue>& args) override;

    private:
        /// @brief Converts an FFIValue to a Python object
        PyObject* convertToPyObject(const FFIValue& value);

        /// @brief Converts a Python object to an FFIValue
        FFIValue convertFromPyObject(PyObject* obj);
    };

} // namespace ffi