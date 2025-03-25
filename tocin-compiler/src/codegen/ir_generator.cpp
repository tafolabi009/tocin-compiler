// File: src/ir/ir_generator.cpp
#include "../ir/ir_generator.h"
#include <stdexcept>
#include <iostream>

namespace ir {

    IRGenerator::IRGenerator(CompilationContext& ctx)
        : context(ctx), builder(std::make_unique<llvm::IRBuilder<>>(*ctx.llvmContext)) {}

    void IRGenerator::generate(std::shared_ptr<ast::Statement> stmt) {
        if (!stmt) return;
        stmt->accept(*this);
    }

    void IRGenerator::pushValue(llvm::Value* value) {
        valueStack.push(value);
    }

    llvm::Value* IRGenerator::popValue() {
        if (valueStack.empty()) return nullptr;
        llvm::Value* value = valueStack.top();
        valueStack.pop();
        return value;
    }

    llvm::Value* IRGenerator::convertFFIValueToLLVM(const ffi::FFIValue& value, const Token& token) {
        switch (value.getType()) {
        case ffi::FFIValueType::Integer:
            return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), value.getInteger());
        case ffi::FFIValueType::Double:
            return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context.llvmContext), value.getDouble());
        case ffi::FFIValueType::Boolean:
            return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context.llvmContext), value.getBoolean());
        case ffi::FFIValueType::String:
            return builder->CreateGlobalStringPtr(value.getString(), "strtmp");
        case ffi::FFIValueType::Null:
            return nullptr;
        case ffi::FFIValueType::Array: {
            auto arr = value.getArray();
            context.errorHandler->reportError(
                "Array FFI return types partially supported", token, error::ErrorSeverity::WARNING
            );
            // Simplified: Return first element or 0 if empty
            return arr.empty() ? llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 0)
                : convertFFIValueToLLVM(arr[0], token);
        }
        case ffi::FFIValueType::Object:
            context.errorHandler->reportError(
                "Object FFI return types not yet supported", token, error::ErrorSeverity::ERROR
            );
            return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 0);
        default:
            context.errorHandler->reportError(
                "Unsupported FFI return type", token, error::ErrorSeverity::ERROR
            );
            return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 0);
        }
    }

    ffi::FFIValue IRGenerator::convertLLVMToFFIValue(llvm::Value* value, const Token& token) {
        if (!value) return ffi::FFIValue();
        if (value->getType()->isIntegerTy(64)) {
            if (auto* constant = llvm::dyn_cast<llvm::ConstantInt>(value)) {
                return ffi::FFIValue(constant->getSExtValue());
            }
        }
        else if (value->getType()->isDoubleTy()) {
            if (auto* constant = llvm::dyn_cast<llvm::ConstantFP>(value)) {
                return ffi::FFIValue(constant->getValue().convertToDouble());
            }
        }
        else if (value->getType()->isIntegerTy(1)) {
            if (auto* constant = llvm::dyn_cast<llvm::ConstantInt>(value)) {
                return ffi::FFIValue(constant->getZExtValue() != 0);
            }
        }
        else if (value->getType()->isPointerTy()) {
            // Assuming string pointer (simplified)
            if (auto* global = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
                if (auto* constant = llvm::dyn_cast<llvm::ConstantDataArray>(global->getInitializer())) {
                    return ffi::FFIValue(constant->getAsCString().str());
                }
            }
        }
        context.errorHandler->reportError(
            "Unsupported LLVM type for FFI conversion", token, error::ErrorSeverity::ERROR
        );
        return ffi::FFIValue(0LL);
    }

    // Statement Visitors
    void IRGenerator::visitExpressionStmt(ast::ExpressionStmt* stmt) {
        stmt->expression->accept(*this);
        popValue(); // Discard result
    }

    void IRGenerator::visitVariableStmt(ast::VariableStmt* stmt) {
        stmt->initializer->accept(*this);
        llvm::Value* value = popValue();
        variables[stmt->name.value] = value;
    }

    void IRGenerator::visitFunctionStmt(ast::FunctionStmt* stmt) {
        context.errorHandler->reportError(
            "Function definitions not fully implemented", stmt->name, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitClassStmt(ast::ClassStmt* stmt) {
        context.errorHandler->reportError(
            "Class definitions not implemented", stmt->name, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitBlockStmt(ast::BlockStmt* stmt) {
        for (auto& s : stmt->statements) {
            s->accept(*this);
        }
    }

    void IRGenerator::visitIfStmt(ast::IfStmt* stmt) {
        context.errorHandler->reportError(
            "If statements not implemented", stmt->token, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitWhileStmt(ast::WhileStmt* stmt) {
        context.errorHandler->reportError(
            "While statements not implemented", stmt->token, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitForStmt(ast::ForStmt* stmt) {
        context.errorHandler->reportError(
            "For statements not implemented", stmt->token, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitReturnStmt(ast::ReturnStmt* stmt) {
        context.errorHandler->reportError(
            "Return statements not implemented", stmt->token, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitImportStmt(ast::ImportStmt* stmt) {
        context.errorHandler->reportError(
            "Import statements not implemented", stmt->token, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitMatchStmt(ast::MatchStmt* stmt) {
        context.errorHandler->reportError(
            "Match statements not implemented", stmt->token, error::ErrorSeverity::WARNING
        );
    }

    // Expression Visitors
    void IRGenerator::visitLiteralExpr(ast::LiteralExpr* expr) {
        switch (expr->literalType) {
        case ast::LiteralExpr::LiteralType::INTEGER:
            pushValue(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext),
                std::stoll(expr->value)));
            break;
        case ast::LiteralExpr::LiteralType::FLOAT:
            pushValue(llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context.llvmContext),
                std::stod(expr->value)));
            break;
        case ast::LiteralExpr::LiteralType::BOOLEAN:
            pushValue(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context.llvmContext),
                expr->value == "True"));
            break;
        case ast::LiteralExpr::LiteralType::STRING:
            pushValue(builder->CreateGlobalStringPtr(expr->value, "strtmp"));
            break;
        case ast::LiteralExpr::LiteralType::NIL:
            pushValue(nullptr);
            break;
        }
    }

    void IRGenerator::visitVariableExpr(ast::VariableExpr* expr) {
        auto it = variables.find(expr->name);
        if (it != variables.end()) {
            pushValue(it->second);
        }
        else {
            context.errorHandler->reportError(
                "Undefined variable: " + expr->name, expr->token, error::ErrorSeverity::ERROR
            );
            pushValue(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 0));
        }
    }

    void IRGenerator::visitAssignExpr(ast::AssignExpr* expr) {
        expr->value->accept(*this);
        llvm::Value* value = popValue();
        variables[expr->name.value] = value;
        pushValue(value);
    }

    void IRGenerator::visitBinaryExpr(ast::BinaryExpr* expr) {
        expr->left->accept(*this);
        llvm::Value* left = popValue();
        expr->right->accept(*this);
        llvm::Value* right = popValue();

        switch (expr->op.type) {
        case TokenType::PLUS:
            pushValue(builder->CreateAdd(left, right, "addtmp"));
            break;
        case TokenType::MINUS:
            pushValue(builder->CreateSub(left, right, "subtmp"));
            break;
        case TokenType::STAR:
            pushValue(builder->CreateMul(left, right, "multmp"));
            break;
        case TokenType::SLASH:
            pushValue(builder->CreateSDiv(left, right, "divtmp"));
            break;
        default:
            context.errorHandler->reportError(
                "Unsupported binary operator", expr->op, error::ErrorSeverity::ERROR
            );
            pushValue(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 0));
        }
    }

    void IRGenerator::visitUnaryExpr(ast::UnaryExpr* expr) {
        expr->right->accept(*this);
        llvm::Value* right = popValue();
        if (expr->op.type == TokenType::MINUS) {
            pushValue(builder->CreateNeg(right, "negtmp"));
        }
        else {
            context.errorHandler->reportError(
                "Unsupported unary operator", expr->op, error::ErrorSeverity::ERROR
            );
            pushValue(right);
        }
    }

    void IRGenerator::visitCallExpr(ast::CallExpr* expr) {
        expr->callee->accept(*this);
        llvm::Value* callee = popValue();

        if (auto varExpr = std::dynamic_pointer_cast<ast::VariableExpr>(expr->callee)) {
            std::string funcName = varExpr->name;
            std::vector<ffi::FFIValue> ffiArgs;
            for (auto& arg : expr->arguments) {
                arg->accept(*this);
                llvm::Value* val = popValue();
                ffiArgs.push_back(convertLLVMToFFIValue(val, expr->token));
            }

            try {
                ffi::FFIValue result;
                if (funcName.substr(0, 3) == "js_") {
                    result = context.javascriptFFI->callJavaScript(funcName.substr(3), ffiArgs);
                }
                else if (funcName.substr(0, 3) == "py_") {
                    result = context.pythonFFI->callPython("user_module", funcName.substr(3), ffiArgs);
                }
                else {
                    result = context.cppFFI->callCpp(funcName, ffiArgs);
                }
                pushValue(convertFFIValueToLLVM(result, expr->token));
            }
            catch (const std::exception& e) {
                context.errorHandler->reportError(
                    "FFI call error: " + std::string(e.what()), expr->token, error::ErrorSeverity::ERROR
                );
                pushValue(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 0));
            }
            return;
        }

        std::vector<llvm::Value*> args;
        for (auto& arg : expr->arguments) {
            arg->accept(*this);
            args.push_back(popValue());
        }
        pushValue(builder->CreateCall(llvm::dyn_cast<llvm::Function>(callee), args, "calltmp"));
    }

    void IRGenerator::visitGetExpr(ast::GetExpr* expr) {
        context.errorHandler->reportError(
            "Get expressions not implemented", expr->token, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitSetExpr(ast::SetExpr* expr) {
        context.errorHandler->reportError(
            "Set expressions not implemented", expr->token, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitGroupingExpr(ast::GroupingExpr* expr) {
        expr->expression->accept(*this);
    }

    void IRGenerator::visitListExpr(ast::ListExpr* expr) {
        context.errorHandler->reportError(
            "List expressions not implemented", expr->token, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitDictionaryExpr(ast::DictionaryExpr* expr) {
        context.errorHandler->reportError(
            "Dictionary expressions not implemented", expr->token, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitLambdaExpr(ast::LambdaExpr* expr) {
        context.errorHandler->reportError(
            "Lambda expressions not implemented", expr->token, error::ErrorSeverity::WARNING
        );
    }

} // namespace ir