#pragma once
#include "ffi_python.h"
#include <stdexcept>
#include <sstream>

namespace {
    void sanitizeFFIArgs(const std::vector<ffi::FFIValue>& args) {
        for (const auto& arg : args) {
            if (auto arr = std::get_if<ffi::FFIValue::ArrayType>(&arg.getValue())) {
                if (arr->size() > 1000) {
                    throw std::runtime_error("FFI input array exceeds maximum allowed size");
                }
            }
            if (auto str = std::get_if<std::string>(&arg.getValue())) {
                if (str->length() > 10000) {
                    throw std::runtime_error("FFI input string exceeds maximum allowed length");
                }
            }
        }
    }
}


namespace ffi {

    PythonFFI::PythonFFI() {
        Py_Initialize();
    }

    PythonFFI::~PythonFFI() {
        Py_Finalize();
    }

    FFIValue PythonFFI::callPython(const std::string& moduleName,
        const std::string& functionName,
        const std::vector<FFIValue>& args) {
        // --- New: Validate FFI inputs ---
        sanitizeFFIArgs(args);

        try {
            PyObject* pModule = PyImport_ImportModule(moduleName.c_str());
            if (!pModule) {
                throw std::runtime_error("Failed to load Python module: " + moduleName);
            }

            PyObject* pFunc = PyObject_GetAttrString(pModule, functionName.c_str());
            if (!pFunc || !PyCallable_Check(pFunc)) {
                Py_XDECREF(pModule);
                throw std::runtime_error("Python function not found: " + functionName);
            }

            PyObject* pArgs = PyTuple_New(args.size());
            for (size_t i = 0; i < args.size(); ++i) {
                PyObject* pValue = convertToPyObject(args[i]);
                PyTuple_SetItem(pArgs, i, pValue); // Steals reference
            }

            PyObject* pResult = PyObject_CallObject(pFunc, pArgs);

            Py_DECREF(pArgs);
            Py_DECREF(pFunc);
            Py_DECREF(pModule);

            if (!pResult) {
                PyErr_Print();
                throw std::runtime_error("Python function call failed");
            }

            FFIValue result = convertFromPyObject(pResult);
            Py_DECREF(pResult);
            return result;
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Python FFI error: " + std::string(e.what()));
        }
    }

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
                Py_RETURN_NONE;
            }
            PyObject* operator()(const FFIValue::ArrayType& arr) const {
                PyObject* listObj = PyList_New(arr.size());
                for (size_t i = 0; i < arr.size(); i++) {
                    PyObject* item = std::visit(Visitor(), arr[i].getValue());
                    PyList_SetItem(listObj, i, item); // Steals reference
                }
                return listObj;
            }
            PyObject* operator()(const FFIValue::ObjectType& obj) const {
                PyObject* dictObj = PyDict_New();
                for (const auto& [key, val] : obj) {
                    PyObject* pyKey = PyUnicode_FromString(key.c_str());
                    PyObject* pyVal = std::visit(Visitor(), val.getValue());
                    PyDict_SetItem(dictObj, pyKey, pyVal);
                    Py_DECREF(pyKey);
                    Py_DECREF(pyVal);
                }
                return dictObj;
            }
        };

        return std::visit(Visitor(), var);
    }

    FFIValue PythonFFI::convertFromPyObject(PyObject* obj) {
        if (obj == Py_None) {
            return FFIValue();
        }
        try {
            if (PyLong_Check(obj)) {
                return FFIValue(static_cast<int64_t>(PyLong_AsLongLong(obj)));
            }
            if (PyFloat_Check(obj)) {
                return FFIValue(PyFloat_AsDouble(obj));
            }
            if (PyBool_Check(obj)) {
                return FFIValue(obj == Py_True);
            }
            if (PyUnicode_Check(obj)) {
                return FFIValue(std::string(PyUnicode_AsUTF8(obj)));
            }
            if (PyList_Check(obj)) {
                FFIValue::ArrayType arr;
                Py_ssize_t len = PyList_Size(obj);
                for (Py_ssize_t i = 0; i < len; i++) {
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
                    std::string keyStr = PyUnicode_AsUTF8(key);
                    dict[keyStr] = convertFromPyObject(value);
                }
                return FFIValue(dict);
            }
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Failed to convert value from Python: " + std::string(e.what()));
        }
        throw std::runtime_error("Unsupported Python value type");
    }

    FFIValue PythonFFI::callJavaScript(const std::string& functionName,
        const std::vector<FFIValue>& args) {
        throw std::runtime_error("JavaScript calls not supported in Python FFI");
    }

    FFIValue PythonFFI::callCpp(const std::string& functionName,
        const std::vector<FFIValue>& args) {
        throw std::runtime_error("C++ calls not supported in Python FFI");
    }

} // namespace ffi
