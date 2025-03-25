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
            if (arr.empty()) return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 0);
            std::vector<llvm::Constant*> elements;
            for (const auto& elem : arr) {
                elements.push_back(llvm::cast<llvm::Constant>(convertFFIValueToLLVM(elem, token)));
            }
            auto arrType = llvm::ArrayType::get(elements[0]->getType(), elements.size());
            return llvm::ConstantArray::get(arrType, elements);
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
        std::vector<llvm::Type*> paramTypes(stmt->parameters.size(), llvm::Type::getInt64Ty(*context.llvmContext));
        llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getInt64Ty(*context.llvmContext), paramTypes, false);
        llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, stmt->name.value, *context.module);
        functions[stmt->name.value] = func;

        llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context.llvmContext, "entry", func);
        builder->SetInsertPoint(entry);

        currentFunction = func;
        auto argIt = func->arg_begin();
        for (const auto& param : stmt->parameters) {
            variables[param.value] = &*argIt++;
        }

        stmt->body->accept(*this);
        if (!builder->GetInsertBlock()->getTerminator()) {
            builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 0));
        }
        currentFunction = nullptr;
    }

    void IRGenerator::visitClassStmt(ast::ClassStmt* stmt) {
        auto structType = llvm::StructType::create(*context.llvmContext, stmt->name.value);
        context.module->getTypeByName(stmt->name.value);
        context.errorHandler->reportError(
            "Class definitions partially implemented (struct created)", stmt->name, error::ErrorSeverity::WARNING
        );
    }

    void IRGenerator::visitBlockStmt(ast::BlockStmt* stmt) {
        for (auto& s : stmt->statements) {
            s->accept(*this);
        }
    }

    void IRGenerator::visitIfStmt(ast::IfStmt* stmt) {
        stmt->condition->accept(*this);
        llvm::Value* cond = popValue();
        cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context.llvmContext), 0), "ifcond");

        llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context.llvmContext, "then", currentFunction);
        llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*context.llvmContext, "else");
        llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context.llvmContext, "merge");

        builder->CreateCondBr(cond, thenBB, stmt->elseBranch ? elseBB : mergeBB);

        builder->SetInsertPoint(thenBB);
        stmt->thenBranch->accept(*this);
        if (!builder->GetInsertBlock()->getTerminator()) {
            builder->CreateBr(mergeBB);
        }

        if (stmt->elseBranch) {
            currentFunction->getBasicBlockList().push_back(elseBB);
            builder->SetInsertPoint(elseBB);
            stmt->elseBranch->accept(*this);
            if (!builder->GetInsertBlock()->getTerminator()) {
                builder->CreateBr(mergeBB);
            }
        }

        currentFunction->getBasicBlockList().push_back(mergeBB);
        builder->SetInsertPoint(mergeBB);
    }

    void IRGenerator::visitWhileStmt(ast::WhileStmt* stmt) {
        llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context.llvmContext, "loop", currentFunction);
        llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*context.llvmContext, "body");
        llvm::BasicBlock* exitBB = llvm::BasicBlock::Create(*context.llvmContext, "exit");

        builder->CreateBr(loopBB);
        currentFunction->getBasicBlockList().push_back(loopBB);
        builder->SetInsertPoint(loopBB);

        stmt->condition->accept(*this);
        llvm::Value* cond = popValue();
        cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context.llvmContext), 0), "whilecond");
        builder->CreateCondBr(cond, bodyBB, exitBB);

        currentFunction->getBasicBlockList().push_back(bodyBB);
        builder->SetInsertPoint(bodyBB);
        stmt->body->accept(*this);
        builder->CreateBr(loopBB);

        currentFunction->getBasicBlockList().push_back(exitBB);
        builder->SetInsertPoint(exitBB);
    }

    void IRGenerator::visitForStmt(ast::ForStmt* stmt) {
        if (stmt->initializer) stmt->initializer->accept(*this);
        llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context.llvmContext, "loop", currentFunction);
        llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*context.llvmContext, "body");
        llvm::BasicBlock* exitBB = llvm::BasicBlock::Create(*context.llvmContext, "exit");

        builder->CreateBr(loopBB);
        currentFunction->getBasicBlockList().push_back(loopBB);
        builder->SetInsertPoint(loopBB);

        stmt->condition->accept(*this);
        llvm::Value* cond = popValue();
        cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context.llvmContext), 0), "forcond");
        builder->CreateCondBr(cond, bodyBB, exitBB);

        currentFunction->getBasicBlockList().push_back(bodyBB);
        builder->SetInsertPoint(bodyBB);
        stmt->body->accept(*this);
        if (stmt->increment) stmt->increment->accept(*this);
        builder->CreateBr(loopBB);

        currentFunction->getBasicBlockList().push_back(exitBB);
        builder->SetInsertPoint(exitBB);
    }

    void IRGenerator::visitReturnStmt(ast::ReturnStmt* stmt) {
        if (stmt->value) {
            stmt->value->accept(*this);
            llvm::Value* retVal = popValue();
            builder->CreateRet(retVal);
        }
        else {
            builder->CreateRetVoid();
        }
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
            "Get expressions not fully implemented", expr->token, error::ErrorSeverity::WARNING
        );
        pushValue(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 0));
    }

    void IRGenerator::visitSetExpr(ast::SetExpr* expr) {
        context.errorHandler->reportError(
            "Set expressions not fully implemented", expr->token, error::ErrorSeverity::WARNING
        );
        expr->value->accept(*this);
        pushValue(popValue());
    }

    void IRGenerator::visitGroupingExpr(ast::GroupingExpr* expr) {
        expr->expression->accept(*this);
    }

    void IRGenerator::visitListExpr(ast::ListExpr* expr) {
        std::vector<llvm::Constant*> elements;
        for (auto& elem : expr->elements) {
            elem->accept(*this);
            elements.push_back(llvm::cast<llvm::Constant>(popValue()));
        }
        auto arrType = llvm::ArrayType::get(llvm::Type::getInt64Ty(*context.llvmContext), elements.size());
        llvm::Value* arr = llvm::ConstantArray::get(arrType, elements);
        pushValue(arr);
    }

    void IRGenerator::visitDictionaryExpr(ast::DictionaryExpr* expr) {
        auto structType = llvm::StructType::create(*context.llvmContext, "dict_" + std::to_string(reinterpret_cast<uintptr_t>(expr)));
        std::vector<llvm::Type*> fieldTypes;
        std::vector<llvm::Constant*> fieldValues;
        for (const auto& [key, val] : expr->pairs) {
            val->accept(*this);
            fieldValues.push_back(llvm::cast<llvm::Constant>(popValue()));
            fieldTypes.push_back(fieldValues.back()->getType());
        }
        structType->setBody(fieldTypes);
        pushValue(llvm::ConstantStruct::get(structType, fieldValues));
    }

    void IRGenerator::visitLambdaExpr(ast::LambdaExpr* expr) {
        std::vector<llvm::Type*> paramTypes(expr->parameters.size(), llvm::Type::getInt64Ty(*context.llvmContext));
        llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getInt64Ty(*context.llvmContext), paramTypes, false);
        llvm::Function* lambda = llvm::Function::Create(funcType, llvm::Function::InternalLinkage, "lambda", *context.module);

        llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context.llvmContext, "entry", lambda);
        builder->SetInsertPoint(entry);

        auto argIt = lambda->arg_begin();
        for (const auto& param : expr->parameters) {
            variables[param.value] = &*argIt++;
        }

        expr->body->accept(*this);
        llvm::Value* retVal = popValue();
        builder->CreateRet(retVal);

        pushValue(lambda);
    }

} // namespace ir