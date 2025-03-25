// File: src/ir/ir_generator.h
#pragma once

#include "../ast/ast.h"
#include "../compiler/compilation_context.h"
#include <llvm/IR/IRBuilder.h>
#include <stack>
#include <unordered_map>

namespace ir {

    class IRGenerator : public ast::Visitor {
    public:
        explicit IRGenerator(CompilationContext& ctx);
        void generate(std::shared_ptr<ast::Statement> stmt);

    private:
        CompilationContext& context;
        std::unique_ptr<llvm::IRBuilder<> > builder;
        std::stack<llvm::Value*> valueStack;
        std::unordered_map<std::string, llvm::Value*> variables;
        std::unordered_map<std::string, llvm::Function*> functions;
        llvm::Function* currentFunction = nullptr;

        void visitExpressionStmt(ast::ExpressionStmt* stmt) override;
        void visitVariableStmt(ast::VariableStmt* stmt) override;
        void visitFunctionStmt(ast::FunctionStmt* stmt) override;
        void visitClassStmt(ast::ClassStmt* stmt) override;
        void visitBlockStmt(ast::BlockStmt* stmt) override;
        void visitIfStmt(ast::IfStmt* stmt) override;
        void visitWhileStmt(ast::WhileStmt* stmt) override;
        void visitForStmt(ast::ForStmt* stmt) override;
        void visitReturnStmt(ast::ReturnStmt* stmt) override;
        void visitImportStmt(ast::ImportStmt* stmt) override;
        void visitMatchStmt(ast::MatchStmt* stmt) override;

        void visitLiteralExpr(ast::LiteralExpr* expr) override;
        void visitVariableExpr(ast::VariableExpr* expr) override;
        void visitAssignExpr(ast::AssignExpr* expr) override;
        void visitBinaryExpr(ast::BinaryExpr* expr) override;
        void visitUnaryExpr(ast::UnaryExpr* expr) override;
        void visitCallExpr(ast::CallExpr* expr) override;
        void visitGetExpr(ast::GetExpr* expr) override;
        void visitSetExpr(ast::SetExpr* expr) override;
        void visitGroupingExpr(ast::GroupingExpr* expr) override;
        void visitListExpr(ast::ListExpr* expr) override;
        void visitDictionaryExpr(ast::DictionaryExpr* expr) override;
        void visitLambdaExpr(ast::LambdaExpr* expr) override;

        void pushValue(llvm::Value* value);
        llvm::Value* popValue();
        llvm::Value* convertFFIValueToLLVM(const ffi::FFIValue& value, const Token& token);
        ffi::FFIValue convertLLVMToFFIValue(llvm::Value* value, const Token& token);
    };

} // namespace ir