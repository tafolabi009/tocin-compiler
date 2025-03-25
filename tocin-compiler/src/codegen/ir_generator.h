// File: src/ir/ir_generator.h
#pragma once

#include "../ast/ast.h"
#include "../compiler/compilation_context.h"
#include <llvm/IR/IRBuilder.h>
#include <stack>
#include <unordered_map>

namespace ir {

    /// @brief Generates LLVM IR from an AST
    class IRGenerator : public ast::Visitor {
    public:
        /// @brief Constructs an IR generator with a compilation context
        explicit IRGenerator(CompilationContext& ctx);

        /// @brief Generates IR for the given statement
        void generate(std::shared_ptr<ast::Statement> stmt);

    private:
        CompilationContext& context;                    ///< Compilation context
        std::unique_ptr<llvm::IRBuilder<> > builder;    ///< LLVM IR builder
        std::stack<llvm::Value*> valueStack;            ///< Stack for expression values
        std::unordered_map<std::string, llvm::Value*> variables; ///< Variable mappings

        // Visitor methods
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

        /// @brief Pushes a value onto the stack
        void pushValue(llvm::Value* value);

        /// @brief Pops a value from the stack
        llvm::Value* popValue();

        /// @brief Converts an FFIValue to an LLVM value
        llvm::Value* convertFFIValueToLLVM(const ffi::FFIValue& value, const Token& token);

        /// @brief Converts an LLVM value to an FFIValue
        ffi::FFIValue convertLLVMToFFIValue(llvm::Value* value, const Token& token);
    };

} // namespace ir