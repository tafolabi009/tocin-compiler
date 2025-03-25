// File: src/ffi/ffi_python.cpp
#include "ffi_python.h"
#include <stdexcept>
#include <sstream>

namespace {

    void sanitizeFFIArgs(const std::vector<ffi::FFIValue>& args, size_t maxArraySize = 1000, size_t maxStringLength = 10000) {
        for (const auto& arg : args) {
            if (arg.getType() == ffi::FFIValueType::Array) {
                auto arr = arg.getArray();
                if (arr.size() > maxArraySize) {
                    throw std::runtime_error("FFI input array exceeds maximum allowed size (" + std::to_string(maxArraySize) + ")");
                }
            }
            else if (arg.getType() == ffi::FFIValueType::String) {
                auto str = arg.getString();
                if (str.length() > maxStringLength) {
                    throw std::runtime_error("FFI input string exceeds maximum allowed length (" + std::to_string(maxStringLength) + ")");
                }
            }
        }
    }

} // anonymous namespace

namespace ffi {

    PythonFFI::PythonFFI() {
        // Rely on CompilationContext to initialize Python
    }

    PythonFFI::~PythonFFI() {
        // Rely on CompilationContext to finalize Python
    }

    FFIValue PythonFFI::callPython(const std::string& moduleName, const std::string& functionName,
        const std::vector<FFIValue>& args) {
        sanitizeFFIArgs(args);

        PyGILState_STATE gstate = PyGILState_Ensure();
        try {
            PyObject* pModule = PyImport_ImportModule(moduleName.c_str());
            if (!pModule) {
                PyErr_Print();
                throw std::runtime_error("Failed to load Python module: " + moduleName);
            }

            PyObject* pFunc = PyObject_GetAttrString(pModule, functionName.c_str());
            if (!pFunc || !PyCallable_Check(pFunc)) {
                Py_XDECREF(pFunc);
                Py_DECREF(pModule);
                throw std::runtime_error("Python function not found or not callable: " + functionName);
            }

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

            PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            Py_DECREF(pFunc);
            Py_DECREF(pModule);

            if (!pResult) {
                PyErr_Print();
                throw std::runtime_error("Python function call failed: " + functionName);
            }

            FFIValue result = convertFromPyObject(pResult);
            Py_DECREF(pResult);
            PyGILState_Release(gstate);
            return result;
        }
        catch (const std::exception& e) {
            PyGILState_Release(gstate);
            throw std::runtime_error("Python FFI error: " + std::string(e.what()));
        }
    }

    PyObject* PythonFFI::convertToPyObject(const FFIValue& value) {
        struct Visitor {
            PyObject* operator()(int64_t val) const { return PyLong_FromLongLong(val); }
            PyObject* operator()(double val) const { return PyFloat_FromDouble(val); }
            PyObject* operator()(bool val) const { return PyBool_FromLong(val ? 1 : 0); }
            PyObject* operator()(const std::string& val) const { return PyUnicode_FromString(val.c_str()); }
            PyObject* operator()(std::nullptr_t) const { Py_INCREF(Py_None); return Py_None; }
            PyObject* operator()(const FFIValue::ArrayType& arr) const {
                PyObject* listObj = PyList_New(arr.size());
                if (!listObj) return nullptr;
                for (size_t i = 0; i < arr.size(); ++i) {
                    PyObject* item = std::visit(*this, arr[i].getValue());
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
                    PyObject* pyVal = std::visit(*this, val.getValue());
                    if (!pyKey || !pyVal || PyDict_SetItem(dictObj, pyKey, pyVal) < 0) {
                        Py_XDECREF(pyKey);
                        Py_XDECREF(pyVal);
                        Py_DECREF(dictObj);
                        return nullptr;
                    }
                    Py_DECREF(pyKey);
                    Py_DECREF(pyVal);
                }
                return dictObj;
            }
        };
        return std::visit(Visitor(), value.getValue());
    }

    FFIValue PythonFFI::convertFromPyObject(PyObject* obj) {
        if (obj == Py_None) return FFIValue();
        if (PyLong_Check(obj)) {
            int overflow;
            long long value = PyLong_AsLongLongAndOverflow(obj, &overflow);
            if (overflow) throw std::runtime_error("Integer overflow in Python long");
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
        if (PyBool_Check(obj)) return FFIValue(obj == Py_True);
        if (PyUnicode_Check(obj)) {
            const char* str = PyUnicode_AsUTF8(obj);
            if (!str) throw std::runtime_error("Failed to convert Python unicode to UTF-8");
            return FFIValue(std::string(str));
        }
        if (PyList_Check(obj)) {
            FFIValue::ArrayType arr;
            Py_ssize_t len = PyList_Size(obj);
            arr.reserve(static_cast<size_t>(len));
            for (Py_ssize_t i = 0; i < len; ++i) {
                PyObject* item = PyList_GetItem(obj, i); // Borrowed reference
                arr.push_back(convertFromPyObject(item));
            }
            return FFIValue(arr);
        }
        if (PyDict_Check(obj)) {
            FFIValue::ObjectType dict;
            PyObject* key, * value;
            Py_ssize_t pos = 0;
            while (PyDict_Next(obj, &pos, &key, &value)) {
                if (!PyUnicode_Check(key)) throw std::runtime_error("Dictionary key must be a string");
                const char* keyStr = PyUnicode_AsUTF8(key);
                if (!keyStr) throw std::runtime_error("Failed to convert dictionary key to UTF-8");
                dict[std::string(keyStr)] = convertFromPyObject(value);
            }
            return FFIValue(dict);
        }
        PyObject* typeStr = PyObject_Str(PyObject_Type(obj));
        std::string typeInfo = PyUnicode_AsUTF8(typeStr) ? PyUnicode_AsUTF8(typeStr) : "unknown_type";
        Py_DECREF(typeStr);
        throw std::runtime_error("Unsupported Python value type: " + typeInfo);
    }

    FFIValue PythonFFI::callJavaScript(const std::string& functionName, const std::vector<FFIValue>& args) {
        throw std::runtime_error("JavaScript calls not supported in Python FFI");
    }

    FFIValue PythonFFI::callCpp(const std::string& functionName, const std::vector<FFIValue>& args) {
        throw std::runtime_error("C++ calls not supported in Python FFI");
    }

} // namespace ffi