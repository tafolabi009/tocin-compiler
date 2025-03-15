#pragma once
#ifdef _WIN32
#define MS_WINDOWS
#endif
#include <python3.12/Python.h>

#include "ffi_interface.h"
#include <vector>
#include <string>

namespace ffi {

    class PythonFFI : public FFIInterface {
    public:
        PythonFFI();
        ~PythonFFI();

        FFIValue callJavaScript(const std::string& functionName,
            const std::vector<FFIValue>& args) override;

        FFIValue callPython(const std::string& moduleName,
            const std::string& functionName,
            const std::vector<FFIValue>& args) override;

        FFIValue callCpp(const std::string& functionName,
            const std::vector<FFIValue>& args) override;

    private:
        PyObject* convertToPyObject(const FFIValue& value);
        FFIValue convertFromPyObject(PyObject* obj);
    };

} // namespace ffi
