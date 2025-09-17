#ifndef TOCIN_FFI_INTERFACE_H
#define TOCIN_FFI_INTERFACE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "ffi_value.h"
#include "../ast/types.h"

namespace ffi {

/**
 * @brief Common FFI interface implemented by specific backends (Python, JS, C++)
 */
class FFIInterface {
public:
    virtual ~FFIInterface() = default;

    // Lifecycle
    virtual bool initialize() = 0;
    virtual void finalize() = 0;
    virtual bool isInitialized() const = 0;

    // Introspection
    virtual std::string getLanguageName() const = 0;
    virtual std::string getVersion() const = 0;

    // Invocation
    virtual FFIValue callFunction(const std::string& functionName, const std::vector<FFIValue>& args) = 0;
    virtual bool hasFunction(const std::string& functionName) const = 0;

    // Modules
    virtual bool loadModule(const std::string& moduleName) = 0;
    virtual bool unloadModule(const std::string& moduleName) = 0;
    virtual bool isModuleLoaded(const std::string& moduleName) const = 0;

    // Conversion
    virtual FFIValue toFFIValue(ast::ValuePtr value) = 0;
    virtual ast::ValuePtr fromFFIValue(const FFIValue& value) = 0;

    // Errors
    virtual bool hasError() const = 0;
    virtual std::string getLastError() const = 0;
    virtual void clearError() = 0;

    // Capabilities
    virtual std::vector<std::string> getSupportedFeatures() const = 0;
    virtual bool supportsFeature(const std::string& feature) const = 0;
};

/**
 * @brief Main FFI manager (lightweight registry)
 */
class FFIManager {
private:
    std::unordered_map<std::string, std::shared_ptr<FFIInterface>> interfaces_;

public:
    FFIManager() = default;
    ~FFIManager() = default;

    void registerInterface(const std::string& language, std::shared_ptr<FFIInterface> iface) {
        interfaces_[language] = std::move(iface);
    }

    std::shared_ptr<FFIInterface> get(const std::string& language) const {
        auto it = interfaces_.find(language);
        return it == interfaces_.end() ? nullptr : it->second;
    }

    std::vector<std::string> getAvailableLanguages() const {
        std::vector<std::string> langs;
        langs.reserve(interfaces_.size());
        for (const auto& kv : interfaces_) langs.push_back(kv.first);
        return langs;
    }
};

} // namespace ffi

#endif // TOCIN_FFI_INTERFACE_H