#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "../ast/ast.h"
#include "../compiler/compilation_context.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <unordered_map>

namespace ir {
    class IRGenerator : public ast::Visitor {
    public:
        explicit IRGenerator(CompilationContext& ctx);
        virtual ~IRGenerator() = default;

        llvm::Value* convertFFIValueToLLVM(const ffi::FFIValue& value, const Token& token);
        ffi::FFIValue convertLLVMToFFIValue(llvm::Value* value, const Token& token);

        // Expression visitors
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

        // Statement visitors
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

    private:
        CompilationContext& context;
        std::unique_ptr<llvm::IRBuilder<>> builder;
        std::unordered_map<std::string, llvm::Value*> variables;
        std::unordered_map<std::string, llvm::Function*> functions;
        llvm::Function* currentFunction = nullptr;
        std::vector<llvm::Value*> valueStack;

        void pushValue(llvm::Value* value);
    };
}

#endif // IR_GENERATOR_H