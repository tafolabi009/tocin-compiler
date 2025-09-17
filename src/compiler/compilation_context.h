#ifndef COMPILATION_CONTEXT_H
#define COMPILATION_CONTEXT_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>

#include "../ast/types.h"

namespace tocin {
namespace compiler {

class CompilationContext {
public:
    // Symbol information
    struct Symbol {
        std::string name;
        ast::TypePtr type;
        bool isConstant;
        bool isGlobal;
        size_t scopeLevel;

        Symbol(const std::string& n, ast::TypePtr t, bool c, bool g)
            : name(n), type(std::move(t)), isConstant(c), isGlobal(g), scopeLevel(0) {}
    };

    // Function/class/trait/module metadata (kept minimal for compilation needs)
    struct FunctionInfo { std::string name; };
    struct ClassInfo { std::string name; };
    struct TraitInfo { std::string name; };
    struct ModuleInfo { std::string name; std::string path; bool isLoaded{false}; };

    struct GenericInstantiation {
        std::string baseName;
        std::vector<ast::TypePtr> typeArguments;
        ast::TypePtr instantiatedType;
    };

public:
    CompilationContext(const std::string& filename);
    ~CompilationContext() = default;

    // File and module management
    const std::string& getFilename() const { return filename_; }
    void setFilename(const std::string& filename) { filename_ = filename; }
    const std::string& getCurrentModule() const { return currentModule_; }
    void setCurrentModule(const std::string& moduleName) { currentModule_ = moduleName; }

    // Hot hybrid/JIT/Optimization flags
    bool isHotHybridEnabled() const { return hotHybridEnabled_; }
    void setHotHybridEnabled(bool enabled) { hotHybridEnabled_ = enabled; }
    bool isJITEnabled() const { return jitEnabled_; }
    void setJITEnabled(bool enabled) { jitEnabled_ = enabled; }
    int getOptimizationLevel() const { return optimizationLevel_; }
    void setOptimizationLevel(int level) { optimizationLevel_ = level; }

    // Feature flags
    bool isFFIEnabled() const { return ffiEnabled_; }
    void setFFIEnabled(bool enabled) { ffiEnabled_ = enabled; }
    bool isConcurrencyEnabled() const { return concurrencyEnabled_; }
    void setConcurrencyEnabled(bool enabled) { concurrencyEnabled_ = enabled; }
    bool isAdvancedFeaturesEnabled() const { return advancedFeaturesEnabled_; }
    void setAdvancedFeaturesEnabled(bool enabled) { advancedFeaturesEnabled_ = enabled; }

    // Scope/symbol table API
    void enterScope();
    void exitScope();
    bool declareSymbol(const std::string& name, ast::TypePtr type, bool isConstant);
    bool declareSymbol(const Symbol& symbol);
    Symbol* lookupSymbol(const std::string& name);
    const Symbol* lookupSymbol(const std::string& name) const;
    bool isSymbolDeclared(const std::string& name) const;

    // Function/Class/Trait registries
    bool declareFunction(const FunctionInfo& function);
    FunctionInfo* lookupFunction(const std::string& name);
    const FunctionInfo* lookupFunction(const std::string& name) const;
    std::vector<FunctionInfo*> lookupOverloadedFunctions(const std::string& name);

    bool declareClass(const ClassInfo& classInfo);
    ClassInfo* lookupClass(const std::string& name);
    const ClassInfo* lookupClass(const std::string& name) const;

    bool declareTrait(const TraitInfo& traitInfo);
    TraitInfo* lookupTrait(const std::string& name);
    const TraitInfo* lookupTrait(const std::string& name) const;

    // Modules
    bool importModule(const std::string& moduleName, const std::string& path);
    ModuleInfo* lookupModule(const std::string& name);
    const ModuleInfo* lookupModule(const std::string& name) const;

    // Generics
    bool registerGenericInstantiation(const GenericInstantiation& instantiation);
    ast::TypePtr lookupGenericInstantiation(const std::string& baseName,
                                            const std::vector<ast::TypePtr>& typeArguments);

    // Dependencies
    void addDependency(const std::string& moduleName);
    const std::unordered_set<std::string>& getDependencies() const { return dependencies_; }

    // Compilation state
    bool isCompiling() const { return isCompiling_; }
    void setCompiling(bool compiling) { isCompiling_ = compiling; }

    // Diagnostics
    void addError(const std::string& message, size_t line = 0, size_t column = 0);
    void addWarning(const std::string& message, size_t line = 0, size_t column = 0);
    void addError(const std::string& error); // legacy/simple variant
    const std::vector<std::string>& getErrors() const { return errors_; }
    const std::vector<std::string>& getWarnings() const { return warnings_; }
    void clearErrors() { errors_.clear(); }

    // Performance tracking
    void startTimer(const std::string& phase);
    double endTimer(const std::string& phase);
    const std::unordered_map<std::string, double>& getTimings() const { return timings_; }

    // Hot reload
    void markForHotReload(const std::string& symbol);
    const std::unordered_set<std::string>& getHotReloadSymbols() const { return hotReloadSymbols_; }
    void clearHotReloadSymbols() { hotReloadSymbols_.clear(); }

    // Thread safety helpers
    void lock() { mutex_.lock(); }
    void unlock() { mutex_.unlock(); }
    std::unique_lock<std::mutex> getLock() { return std::unique_lock<std::mutex>(mutex_); }

private:
    std::string filename_;
    std::string currentModule_;

    // Flags
    bool hotHybridEnabled_ = true;
    bool jitEnabled_ = true;
    int optimizationLevel_ = 2;
    bool ffiEnabled_ = true;
    bool concurrencyEnabled_ = true;
    bool advancedFeaturesEnabled_ = true;

    // State
    bool isCompiling_ = false;

    // Global symbol table for external pointers
    std::unordered_map<std::string, void*> symbols_;

    // Scoped symbol tables for semantic analysis
    size_t currentScopeLevel_ = 0;
    std::unordered_map<size_t, std::unordered_map<std::string, Symbol>> symbolTables_;

    // Registries
    std::unordered_map<std::string, FunctionInfo> functions_;
    std::unordered_map<std::string, std::vector<FunctionInfo>> overloadedFunctions_;
    std::unordered_map<std::string, ClassInfo> classes_;
    std::unordered_map<std::string, TraitInfo> traits_;
    std::unordered_map<std::string, ModuleInfo> modules_;
    std::vector<GenericInstantiation> genericInstantiations_;

    // Dependencies
    std::unordered_set<std::string> dependencies_;

    // Diagnostics
    std::vector<std::string> errors_;
    std::vector<std::string> warnings_;

    // Timers
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> timers_;
    std::unordered_map<std::string, double> timings_;

    // Hot reload
    std::unordered_set<std::string> hotReloadSymbols_;

    // Thread safety
    mutable std::mutex mutex_;
};

} // namespace compiler
} // namespace tocin

#endif // COMPILATION_CONTEXT_H
