#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
#include "../ast/ast.h"

// LLVM headers
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/FileSystem.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class IRGenerator : public ast::Visitor {
public:
    IRGenerator();
    void generate(std::shared_ptr<ast::Statement> ast, const std::string& outputFile);

private:
    // LLVM context, module, and IR builder
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    // Symbol table for variables and functions
    class SymbolTable {
    public:
        SymbolTable() : parent(nullptr) {}
        SymbolTable(SymbolTable* parent) : parent(parent) {}

        void define(const std::string& name, llvm::Value* value) {
            values[name] = value;
        }
        llvm::Value* get(const std::string& name) {
            if (values.find(name) != values.end()) {
                return values[name];
            }
            if (parent) {
                return parent->get(name);
            }
            return nullptr;
        }
        SymbolTable* parent;
    private:
        std::unordered_map<std::string, llvm::Value*> values;
    };

    SymbolTable* currentSymbolTable;
    std::stack<llvm::Value*> valueStack;

    // Current function being generated
    llvm::Function* currentFunction;

    // Helper methods
    void beginScope();
    void endScope();
    llvm::Type* toLLVMType(std::shared_ptr<ast::Type> type);
    void pushValue(llvm::Value* value);
    llvm::Value* popValue();

    // Visitor implementations for expressions
    void visitBinaryExpr(ast::BinaryExpr* expr) override;
    void visitGroupingExpr(ast::GroupingExpr* expr) override;
    void visitLiteralExpr(ast::LiteralExpr* expr) override;
    void visitUnaryExpr(ast::UnaryExpr* expr) override;
    void visitVariableExpr(ast::VariableExpr* expr) override;
    void visitAssignExpr(ast::AssignExpr* expr) override;
    void visitCallExpr(ast::CallExpr* expr) override;
    void visitGetExpr(ast::GetExpr* expr) override;
    void visitSetExpr(ast::SetExpr* expr) override;
    void visitListExpr(ast::ListExpr* expr) override;
    void visitDictionaryExpr(ast::DictionaryExpr* expr) override;
    void visitLambdaExpr(ast::LambdaExpr* expr) override;

    // Visitor implementations for statements
    void visitExpressionStmt(ast::ExpressionStmt* stmt) override;
    void visitVariableStmt(ast::VariableStmt* stmt) override;
    void visitBlockStmt(ast::BlockStmt* stmt) override;
    void visitIfStmt(ast::IfStmt* stmt) override;
    void visitWhileStmt(ast::WhileStmt* stmt) override;
    void visitForStmt(ast::ForStmt* stmt) override;
    void visitFunctionStmt(ast::FunctionStmt* stmt) override;
    void visitReturnStmt(ast::ReturnStmt* stmt) override;
    void visitClassStmt(ast::ClassStmt* stmt) override;
    void visitImportStmt(ast::ImportStmt* stmt) override;
    void visitMatchStmt(ast::MatchStmt* stmt) override;

    // Additional code generation methods
    void generateGlobalInitialization();
    void generateStandardLibraryFunctions();
    bool verifyAndWriteOutput(const std::string& outputFile);
};
