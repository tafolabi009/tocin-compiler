
#ifdef _WIN32
#define MS_WINDOWS
#endif
#include <Python.h>

#include "ffi_python.h"
#include <stdexcept>
#include <sstream>

// Removed the problematic unistd.h include and improved platform-specific handling
// No need for unistd.h in this file as it's not being used

namespace {
    // Sanitize FFI arguments to enforce size limits
    void sanitizeFFIArgs(const std::vector<ffi::FFIValue>& args) {
        for (const auto& arg : args) {
            if (auto arr = std::get_if<ffi::FFIValue::ArrayType>(&arg.getValue())) {
                if (arr->size() > 1000) {
                    throw std::runtime_error("FFI input array exceeds maximum allowed size");
                }
            }
            else if (auto str = std::get_if<std::string>(&arg.getValue())) {
                if (str->length() > 10000) {
                    throw std::runtime_error("FFI input string exceeds maximum allowed length");
                }
            }
        }
    }
}

namespace ffi {

    // Constructor: Initialize Python interpreter
    PythonFFI::PythonFFI() {
        if (!Py_IsInitialized()) {
            Py_Initialize();
        }
    }

    // Destructor: Finalize Python interpreter
    PythonFFI::~PythonFFI() {
        // Changed to conditional finalization to prevent crashes if Python
        // was initialized elsewhere in the application
        if (Py_IsInitialized()) {
            // Release the GIL before finalization if we're in a threaded environment
            PyGILState_Ensure();
            Py_Finalize();
        }
    }

    // Call a Python function with given module, function name, and arguments
    FFIValue PythonFFI::callPython(const std::string& moduleName,
        const std::string& functionName,
        const std::vector<FFIValue>& args) {
        // Validate FFI inputs
        sanitizeFFIArgs(args);

        // Make sure we have the GIL when calling Python code
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            // Import the Python module
            PyObject* pModule = PyImport_ImportModule(moduleName.c_str());
            if (!pModule) {
                PyErr_Print(); // Print Python error information
                throw std::runtime_error("Failed to load Python module: " + moduleName);
            }

            // Get the function from the module
            PyObject* pFunc = PyObject_GetAttrString(pModule, functionName.c_str());
            if (!pFunc || !PyCallable_Check(pFunc)) {
                Py_XDECREF(pFunc); // Safe decref - handles nullptr
                Py_DECREF(pModule);
                throw std::runtime_error("Python function not found or not callable: " + functionName);
            }

            // Create a tuple for function arguments
            PyObject* pArgs = PyTuple_New(args.size());
            for (size_t i = 0; i < args.size(); ++i) {
                PyObject* pValue = convertToPyObject(args[i]);
                if (!pValue) {
                    Py_DECREF(pArgs);
                    Py_DECREF(pFunc);
                    Py_DECREF(pModule);
                    throw std::runtime_error("Failed to convert argument " + std::to_string(i) + " to Python object");
                }
                PyTuple_SetItem(pArgs, i, pValue); // Steals reference
            }

            // Call the Python function
            PyObject* pResult = PyObject_CallObject(pFunc, pArgs);

            // Clean up Python objects
            Py_DECREF(pArgs);
            Py_DECREF(pFunc);
            Py_DECREF(pModule);

            if (!pResult) {
                PyErr_Print();
                throw std::runtime_error("Python function call failed");
            }

            // Convert result back to FFIValue
            FFIValue result = convertFromPyObject(pResult);
            Py_DECREF(pResult);
            
            // Release the GIL
            PyGILState_Release(gstate);
            
            return result;
        }
        catch (const std::exception& e) {
            // Release the GIL before rethrowing the exception
            PyGILState_Release(gstate);
            throw std::runtime_error("Python FFI error: " + std::string(e.what()));
        }
    }

    // Convert FFIValue to Python object
    PyObject* PythonFFI::convertToPyObject(const FFIValue& value) {
        const auto& var = value.getValue();
        struct Visitor {
            PyObject* operator()(int64_t val) const {
                return PyLong_FromLongLong(val);
            }
            PyObject* operator()(double val) const {
                return PyFloat_FromDouble(val);
            }
            PyObject* operator()(bool val) const {
                return PyBool_FromLong(val ? 1 : 0);
            }
            PyObject* operator()(const std::string& val) const {
                return PyUnicode_FromString(val.c_str());
            }
            PyObject* operator()(std::nullptr_t) const {
                Py_INCREF(Py_None); // Increment reference count before returning
                return Py_None;
            }
            PyObject* operator()(const FFIValue::ArrayType& arr) const {
                PyObject* listObj = PyList_New(arr.size());
                if (!listObj) return nullptr;
                
                for (size_t i = 0; i < arr.size(); i++) {
                    PyObject* item = std::visit(Visitor(), arr[i].getValue());
                    if (!item) {
                        Py_DECREF(listObj);
                        return nullptr;
                    }
                    PyList_SetItem(listObj, i, item); // Steals reference
                }
                return listObj;
            }
            PyObject* operator()(const FFIValue::ObjectType& obj) const {
                PyObject* dictObj = PyDict_New();
                if (!dictObj) return nullptr;
                
                for (const auto& [key, val] : obj) {
                    PyObject* pyKey = PyUnicode_FromString(key.c_str());
                    if (!pyKey) {
                        Py_DECREF(dictObj);
                        return nullptr;
                    }
                    
                    PyObject* pyVal = std::visit(Visitor(), val.getValue());
                    if (!pyVal) {
                        Py_DECREF(pyKey);
                        Py_DECREF(dictObj);
                        return nullptr;
                    }
                    
                    if (PyDict_SetItem(dictObj, pyKey, pyVal) < 0) {
                        Py_DECREF(pyKey);
                        Py_DECREF(pyVal);
                        Py_DECREF(dictObj);
                        return nullptr;
                    }
                    Py_DECREF(pyKey);
                    Py_DECREF(pyVal);
                }
                return dictObj;
            }
        };

        return std::visit(Visitor(), var);
    }

    // Convert Python object back to FFIValue
    FFIValue PythonFFI::convertFromPyObject(PyObject* obj) {
        if (obj == Py_None) {
            return FFIValue();
        }
        
        try {
            if (PyLong_Check(obj)) {
                long long value = PyLong_AsLongLong(obj);
                if (value == -1 && PyErr_Occurred()) {
                    PyErr_Clear();
                    throw std::runtime_error("Integer overflow in Python long");
                }
                return FFIValue(static_cast<int64_t>(value));
            }
            if (PyFloat_Check(obj)) {
                double value = PyFloat_AsDouble(obj);
                if (value == -1.0 && PyErr_Occurred()) {
                    PyErr_Clear();
                    throw std::runtime_error("Error converting Python float");
                }
                return FFIValue(value);
            }
            if (PyBool_Check(obj)) {
                return FFIValue(obj == Py_True);
            }
            if (PyUnicode_Check(obj)) {
                const char* str = PyUnicode_AsUTF8(obj);
                if (!str) {
                    PyErr_Clear();
                    throw std::runtime_error("Failed to convert Python unicode to UTF-8");
                }
                return FFIValue(std::string(str));
            }
            if (PyList_Check(obj)) {
                FFIValue::ArrayType arr;
                Py_ssize_t len = PyList_Size(obj);
                arr.reserve(static_cast<size_t>(len)); // Optimize memory allocation
                
                for (Py_ssize_t i = 0; i < len; i++) {
                    PyObject* item = PyList_GetItem(obj, i); // Borrowed reference
                    if (!item) {
                        throw std::runtime_error("Failed to get list item at index " + std::to_string(i));
                    }
                    arr.push_back(convertFromPyObject(item));
                }
                return FFIValue(arr);
            }
            if (PyDict_Check(obj)) {
                FFIValue::ObjectType dict;
                PyObject* key, * value;
                Py_ssize_t pos = 0;
                
                while (PyDict_Next(obj, &pos, &key, &value)) {
                    // Ensure key is a string
                    if (!PyUnicode_Check(key)) {
                        throw std::runtime_error("Dictionary key must be a string");
                    }
                    
                    const char* keyStr = PyUnicode_AsUTF8(key);
                    if (!keyStr) {
                        PyErr_Clear();
                        throw std::runtime_error("Failed to convert dictionary key to UTF-8");
                    }
                    
                    dict[std::string(keyStr)] = convertFromPyObject(value);
                }
                return FFIValue(dict);
            }
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Failed to convert value from Python: " + std::string(e.what()));
        }
        
        // Get Python object type name for better error message
        PyObject* type = PyObject_Type(obj);
        PyObject* typeStr = PyObject_Str(type);
        const char* typeCharStr = PyUnicode_AsUTF8(typeStr);
        std::string typeInfo = typeCharStr ? std::string(typeCharStr) : "unknown_type";
        Py_XDECREF(typeStr);
        Py_XDECREF(type);
        
        throw std::runtime_error("Unsupported Python value type: " + typeInfo);
    }

    // Placeholder for JavaScript calls (not supported)
    FFIValue PythonFFI::callJavaScript(const std::string& functionName,
        const std::vector<FFIValue>& args) {
        throw std::runtime_error("JavaScript calls not supported in Python FFI");
    }

    // Placeholder for C++ calls (not supported)
    FFIValue PythonFFI::callCpp(const std::string& functionName,
        const std::vector<FFIValue>& args) {
        throw std::runtime_error("C++ calls not supported in Python FFI");
    }

} // namespace ffi
