#include "ir_generator.h"
#include "../ffi/ffi_value.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>

namespace ir {
    IRGenerator::IRGenerator(CompilationContext& ctx)
        : context(ctx), builder(std::make_unique<llvm::IRBuilder<>>(*ctx.llvmContext)) {}

    llvm::Value* IRGenerator::convertFFIValueToLLVM(const ffi::FFIValue& value, const Token& token) {
        if (std::holds_alternative<int64_t>(value.data)) {
            return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), std::get<int64_t>(value.data));
        }
        else if (std::holds_alternative<double>(value.data)) {
            return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context.llvmContext), std::get<double>(value.data));
        }
        else if (std::holds_alternative<bool>(value.data)) {
            return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context.llvmContext), std::get<bool>(value.data));
        }
        else if (std::holds_alternative<std::string>(value.data)) {
            return builder->CreateGlobalString(std::get<std::string>(value.data), "strtmp");
        }
        context.errorHandler->reportError("Unsupported FFI value type", token);
        return nullptr;
    }

    ffi::FFIValue IRGenerator::convertLLVMToFFIValue(llvm::Value* value, const Token& token) {
        if (auto* intVal = llvm::dyn_cast<llvm::ConstantInt>(value)) {
            if (intVal->getType()->isIntegerTy(1)) {
                return ffi::FFIValue(static_cast<bool>(intVal->getZExtValue()));
            }
            else {
                return ffi::FFIValue(static_cast<int64_t>(intVal->getSExtValue()));
            }
        }
        else if (auto* fpVal = llvm::dyn_cast<llvm::ConstantFP>(value)) {
            return ffi::FFIValue(fpVal->getValueAPF().convertToDouble());
        }
        else if (auto* globalVal = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
            return ffi::FFIValue(globalVal->getName().str());
        }
        context.errorHandler->reportError("Cannot convert LLVM value to FFI", token);
        return ffi::FFIValue();
    }

    void IRGenerator::pushValue(llvm::Value* value) {
        valueStack.push_back(value);
    }

    // Expression Visitors (unchanged except where noted)
    void IRGenerator::visitBinaryExpr(ast::BinaryExpr* expr) {
        expr->left->accept(*this);
        llvm::Value* left = valueStack.back();
        valueStack.pop_back();

        expr->right->accept(*this);
        llvm::Value* right = valueStack.back();
        valueStack.pop_back();

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
            context.errorHandler->reportError("Unsupported binary operator", expr->op);
        }
    }

    void IRGenerator::visitGroupingExpr(ast::GroupingExpr* expr) {
        expr->expression->accept(*this);
    }

    void IRGenerator::visitLiteralExpr(ast::LiteralExpr* expr) {
        switch (expr->literalType) {
        case ast::LiteralExpr::LiteralType::INTEGER:
            pushValue(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), std::stoll(expr->value)));
            break;
        case ast::LiteralExpr::LiteralType::FLOAT:
            pushValue(llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context.llvmContext), std::stod(expr->value)));
            break;
        case ast::LiteralExpr::LiteralType::BOOLEAN:
            pushValue(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context.llvmContext), expr->value == "true"));
            break;
        case ast::LiteralExpr::LiteralType::STRING:
            pushValue(builder->CreateGlobalString(expr->value, "strtmp"));
            break;
        case ast::LiteralExpr::LiteralType::NIL:
            pushValue(llvm::Constant::getNullValue(llvm::Type::getInt64Ty(*context.llvmContext)));
            break;
        }
    }

    void IRGenerator::visitUnaryExpr(ast::UnaryExpr* expr) {
        expr->right->accept(*this);
        llvm::Value* right = valueStack.back();
        valueStack.pop_back();

        if (expr->op.type == TokenType::MINUS) {
            pushValue(builder->CreateNeg(right, "negtmp"));
        }
        else if (expr->op.type == TokenType::BANG) {
            pushValue(builder->CreateNot(right, "nottmp"));
        }
        else {
            context.errorHandler->reportError("Unsupported unary operator", expr->op);
        }
    }

    void IRGenerator::visitVariableExpr(ast::VariableExpr* expr) {
        auto it = variables.find(expr->name);
        if (it != variables.end()) {
            pushValue(it->second);
        }
        else {
            context.errorHandler->reportError("Variable not found: " + expr->name, expr->token);
        }
    }

    void IRGenerator::visitAssignExpr(ast::AssignExpr* expr) {
        expr->value->accept(*this);
        variables[expr->name] = valueStack.back(); // Keep value on stack for expression result
    }

    void IRGenerator::visitCallExpr(ast::CallExpr* expr) {
        expr->callee->accept(*this);
        llvm::Value* callee = valueStack.back();
        valueStack.pop_back();

        std::vector<llvm::Value*> args;
        for (const auto& arg : expr->arguments) {
            arg->accept(*this);
            args.push_back(valueStack.back());
            valueStack.pop_back();
        }

        if (auto* func = llvm::dyn_cast<llvm::Function>(callee)) {
            pushValue(builder->CreateCall(func, args));
        }
        else {
            context.errorHandler->reportError("Callee is not a function", expr->token);
        }
    }

    void IRGenerator::visitGetExpr(ast::GetExpr* expr) {
        expr->object->accept(*this);
        llvm::Value* object = valueStack.back();
        valueStack.pop_back();

        if (auto* structType = llvm::dyn_cast<llvm::StructType>(object->getType()->isPointerTy() ? object->getType()->getPointerElementType() : object->getType())) {
            // Assume the struct has named fields; map field name to index (simplified)
            unsigned index = 0; // Placeholder: need a way to map name to index
            for (unsigned i = 0; i < structType->getNumElements(); ++i) {
                if (structType->getName().str().find(expr->name) != std::string::npos) {
                    index = i;
                    break;
                }
            }
            std::vector<llvm::Value*> indices = {
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context.llvmContext), 0),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context.llvmContext), index)
            };
            llvm::Value* gep = builder->CreateGEP(structType, object, indices, "gettmp");
            pushValue(builder->CreateLoad(structType->getElementType(index), gep, "loadtmp"));
        }
        else {
            context.errorHandler->reportError("GetExpr: Object is not a struct", expr->token);
        }
    }

    void IRGenerator::visitSetExpr(ast::SetExpr* expr) {
        expr->object->accept(*this);
        llvm::Value* object = valueStack.back();
        valueStack.pop_back();

        expr->value->accept(*this);
        llvm::Value* value = valueStack.back();
        valueStack.pop_back();

        if (auto* structType = llvm::dyn_cast<llvm::StructType>(object->getType()->isPointerTy() ? object->getType()->getPointerElementType() : object->getType())) {
            unsigned index = 0; // Placeholder: need a way to map name to index
            for (unsigned i = 0; i < structType->getNumElements(); ++i) {
                if (structType->getName().str().find(expr->name) != std::string::npos) {
                    index = i;
                    break;
                }
            }
            std::vector<llvm::Value*> indices = {
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context.llvmContext), 0),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context.llvmContext), index)
            };
            llvm::Value* gep = builder->CreateGEP(structType, object, indices, "settmp");
            builder->CreateStore(value, gep);
            pushValue(value); // Return the assigned value
        }
        else {
            context.errorHandler->reportError("SetExpr: Object is not a struct", expr->token);
        }
    }

    void IRGenerator::visitListExpr(ast::ListExpr* expr) {
        std::vector<llvm::Value*> elements;
        for (const auto& elem : expr->elements) {
            elem->accept(*this);
            elements.push_back(valueStack.back());
            valueStack.pop_back();
        }
        auto arrType = llvm::ArrayType::get(llvm::Type::getInt64Ty(*context.llvmContext), elements.size());
        pushValue(llvm::ConstantArray::get(arrType, elements));
    }

    void IRGenerator::visitDictionaryExpr(ast::DictionaryExpr* expr) {
        auto structType = llvm::StructType::create(*context.llvmContext, "dict_" + std::to_string(reinterpret_cast<uintptr_t>(expr)));
        std::vector<llvm::Type*> fieldTypes;
        std::vector<llvm::Value*> values;

        for (const auto& [key, val] : expr->entries) {
            key->accept(*this);
            values.push_back(valueStack.back());
            valueStack.pop_back();
            val->accept(*this);
            values.push_back(valueStack.back());
            valueStack.pop_back();
            fieldTypes.push_back(llvm::Type::getInt64Ty(*context.llvmContext)); // Simplification
            fieldTypes.push_back(llvm::Type::getInt64Ty(*context.llvmContext));
        }

        structType->setBody(fieldTypes);
        pushValue(llvm::ConstantStruct::get(structType, values));
    }

    void IRGenerator::visitLambdaExpr(ast::LambdaExpr* expr) {
        std::vector<llvm::Type*> paramTypes(expr->parameters.size(), llvm::Type::getInt64Ty(*context.llvmContext));
        llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getInt64Ty(*context.llvmContext), paramTypes, false);
        llvm::Function* lambda = llvm::Function::Create(funcType, llvm::Function::InternalLinkage, "lambda", *context.module);

        llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*context.llvmContext, "entry", lambda);
        builder->SetInsertPoint(entryBB);

        auto argIt = lambda->arg_begin();
        for (const auto& param : expr->parameters) {
            variables[param.name] = &*argIt++;
        }

        expr->body->accept(*this);
        if (valueStack.empty()) {
            builder->CreateRetVoid();
        }
        else {
            builder->CreateRet(valueStack.back());
            valueStack.pop_back();
        }

        pushValue(lambda);
    }

    // Statement Visitors
    void IRGenerator::visitExpressionStmt(ast::ExpressionStmt* stmt) {
        stmt->expression->accept(*this);
        valueStack.clear(); // Discard expression result
    }

    void IRGenerator::visitVariableStmt(ast::VariableStmt* stmt) {
        if (stmt->initializer) {
            stmt->initializer->accept(*this);
            variables[stmt->name] = valueStack.back();
            valueStack.pop_back();
        }
    }

    void IRGenerator::visitBlockStmt(ast::BlockStmt* stmt) {
        for (const auto& s : stmt->statements) {
            s->accept(*this);
        }
    }

    void IRGenerator::visitIfStmt(ast::IfStmt* stmt) {
        stmt->condition->accept(*this);
        llvm::Value* cond = valueStack.back();
        valueStack.pop_back();

        cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context.llvmContext), 0), "ifcond");

        llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context.llvmContext, "then", currentFunction);
        llvm::BasicBlock* elseBB = stmt->elseBranch ? llvm::BasicBlock::Create(*context.llvmContext, "else") : nullptr;
        llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context.llvmContext, "ifmerge");

        builder->CreateCondBr(cond, thenBB, elseBB ? elseBB : mergeBB);
        builder->SetInsertPoint(thenBB);

        stmt->thenBranch->accept(*this);
        builder->CreateBr(mergeBB);

        if (elseBB) {
            currentFunction->insert(currentFunction->end(), elseBB);
            builder->SetInsertPoint(elseBB);
            stmt->elseBranch->accept(*this);
            builder->CreateBr(mergeBB);
        }

        currentFunction->insert(currentFunction->end(), mergeBB);
        builder->SetInsertPoint(mergeBB);
    }

    void IRGenerator::visitWhileStmt(ast::WhileStmt* stmt) {
        llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context.llvmContext, "loop", currentFunction);
        llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*context.llvmContext, "body");
        llvm::BasicBlock* exitBB = llvm::BasicBlock::Create(*context.llvmContext, "exit");

        builder->CreateBr(loopBB);
        builder->SetInsertPoint(loopBB);

        stmt->condition->accept(*this);
        llvm::Value* cond = valueStack.back();
        valueStack.pop_back();

        cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context.llvmContext), 0), "whilecond");
        builder->CreateCondBr(cond, bodyBB, exitBB);

        currentFunction->insert(currentFunction->end(), bodyBB);
        builder->SetInsertPoint(bodyBB);

        stmt->body->accept(*this);
        builder->CreateBr(loopBB);

        currentFunction->insert(currentFunction->end(), exitBB);
        builder->SetInsertPoint(exitBB);
    }

    void IRGenerator::visitForStmt(ast::ForStmt* stmt) {
        stmt->iterable->accept(*this);
        llvm::Value* iterable = valueStack.back();
        valueStack.pop_back();

        llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context.llvmContext, "loop", currentFunction);
        llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*context.llvmContext, "body");
        llvm::BasicBlock* exitBB = llvm::BasicBlock::Create(*context.llvmContext, "exit");

        // Allocate iterator variable
        llvm::Value* iterVar = builder->CreateAlloca(llvm::Type::getInt64Ty(*context.llvmContext), nullptr, stmt->variable + "_iter");
        variables[stmt->variable] = iterVar;

        if (auto* arrayType = llvm::dyn_cast<llvm::ArrayType>(iterable->getType())) {
            // List iteration
            llvm::Value* index = builder->CreateAlloca(llvm::Type::getInt64Ty(*context.llvmContext), nullptr, "index");
            builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 0), index);

            builder->CreateBr(loopBB);
            builder->SetInsertPoint(loopBB);

            llvm::Value* idxVal = builder->CreateLoad(llvm::Type::getInt64Ty(*context.llvmContext), index, "idx");
            llvm::Value* size = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), arrayType->getNumElements());
            llvm::Value* cond = builder->CreateICmpSLT(idxVal, size, "forcond");
            builder->CreateCondBr(cond, bodyBB, exitBB);

            currentFunction->insert(currentFunction->end(), bodyBB);
            builder->SetInsertPoint(bodyBB);

            std::vector<llvm::Value*> indices = {
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context.llvmContext), 0),
                idxVal
            };
            llvm::Value* elemPtr = builder->CreateGEP(arrayType, iterable, indices, "elemtmp");
            llvm::Value* elem = builder->CreateLoad(arrayType->getElementType(), elemPtr, stmt->variable);
            builder->CreateStore(elem, iterVar);

            stmt->body->accept(*this);

            llvm::Value* nextIdx = builder->CreateAdd(idxVal, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 1), "nextidx");
            builder->CreateStore(nextIdx, index);
            builder->CreateBr(loopBB);
        }
        else if (auto* callExpr = dynamic_cast<ast::CallExpr*>(stmt->iterable.get())) {
            // Assume a range-like call (e.g., range(0, n))
            if (callExpr->arguments.size() == 2) {
                callExpr->arguments[0]->accept(*this);
                llvm::Value* start = valueStack.back();
                valueStack.pop_back();
                callExpr->arguments[1]->accept(*this);
                llvm::Value* end = valueStack.back();
                valueStack.pop_back();

                builder->CreateStore(start, iterVar);
                builder->CreateBr(loopBB);
                builder->SetInsertPoint(loopBB);

                llvm::Value* current = builder->CreateLoad(llvm::Type::getInt64Ty(*context.llvmContext), iterVar, "current");
                llvm::Value* cond = builder->CreateICmpSLT(current, end, "forcond");
                builder->CreateCondBr(cond, bodyBB, exitBB);

                currentFunction->insert(currentFunction->end(), bodyBB);
                builder->SetInsertPoint(bodyBB);

                stmt->body->accept(*this);

                llvm::Value* nextVal = builder->CreateAdd(current, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context.llvmContext), 1), "nextval");
                builder->CreateStore(nextVal, iterVar);
                builder->CreateBr(loopBB);
            }
            else {
                context.errorHandler->reportError("ForStmt: Unsupported range call", stmt->token);
            }
        }
        else {
            context.errorHandler->reportError("ForStmt: Unsupported iterable type", stmt->token);
        }

        currentFunction->insert(currentFunction->end(), exitBB);
        builder->SetInsertPoint(exitBB);
    }

    void IRGenerator::visitFunctionStmt(ast::FunctionStmt* stmt) {
        std::vector<llvm::Type*> paramTypes;
        for (const auto& param : stmt->parameters) {
            paramTypes.push_back(llvm::Type::getInt64Ty(*context.llvmContext)); // Simplification
        }
        llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(*context.llvmContext), paramTypes, false);
        currentFunction = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, stmt->name, *context.module);
        functions[stmt->name] = currentFunction;

        llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*context.llvmContext, "entry", currentFunction);
        builder->SetInsertPoint(entryBB);

        auto argIt = currentFunction->arg_begin();
        for (const auto& param : stmt->parameters) {
            variables[param.name] = &*argIt++;
        }

        stmt->body->accept(*this);
        if (valueStack.empty()) {
            builder->CreateRetVoid();
        }
        else {
            builder->CreateRet(valueStack.back());
            valueStack.pop_back();
        }
    }

    void IRGenerator::visitReturnStmt(ast::ReturnStmt* stmt) {
        if (stmt->value) {
            stmt->value->accept(*this);
            builder->CreateRet(valueStack.back());
            valueStack.pop_back();
        }
        else {
            builder->CreateRetVoid();
        }
    }

    void IRGenerator::visitClassStmt(ast::ClassStmt* stmt) {
        auto structType = llvm::StructType::create(*context.llvmContext, stmt->name);
        std::vector<llvm::Type*> fieldTypes;
        for (const auto& field : stmt->fields) {
            fieldTypes.push_back(llvm::Type::getInt64Ty(*context.llvmContext)); // Simplification
        }
        structType->setBody(fieldTypes);
        context.module->getOrInsertGlobal(stmt->name, structType);

        for (const auto& method : stmt->methods) {
            method->accept(*this);
        }
    }

    void IRGenerator::visitImportStmt(ast::ImportStmt* stmt) {
        // Simplified: Treat as an external function declaration for now
        std::string moduleFuncName = stmt->module + "_init";
        llvm::FunctionType* importType = llvm::FunctionType::get(llvm::Type::getVoidTy(*context.llvmContext), false);
        llvm::Function* importFunc = llvm::Function::Create(importType, llvm::Function::ExternalLinkage, moduleFuncName, *context.module);

        for (const auto& [name, alias] : stmt->imports) {
            std::string funcName = stmt->module + "_" + name;
            llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getInt64Ty(*context.llvmContext), false); // Placeholder
            llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, *context.module);
            functions[alias.empty() ? name : alias] = func;
        }

        builder->CreateCall(importFunc);
    }

    void IRGenerator::visitMatchStmt(ast::MatchStmt* stmt) {
        stmt->value->accept(*this);
        llvm::Value* matchValue = valueStack.back();
        valueStack.pop_back();

        llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context.llvmContext, "matchmerge");
        std::vector<llvm::BasicBlock*> caseBBs;
        std::vector<llvm::Value*> conditions;

        // Generate case blocks
        for (size_t i = 0; i < stmt->cases.size(); ++i) {
            auto* caseBB = llvm::BasicBlock::Create(*context.llvmContext, "case" + std::to_string(i), currentFunction);
            caseBBs.push_back(caseBB);

            builder->SetInsertPoint(caseBB);
            stmt->cases[i].pattern->accept(*this);
            llvm::Value* patternValue = valueStack.back();
            valueStack.pop_back();

            llvm::Value* cond = builder->CreateICmpEQ(matchValue, patternValue, "matchcond" + std::to_string(i));
            conditions.push_back(cond);

            stmt->cases[i].body->accept(*this);
            builder->CreateBr(mergeBB);
        }

        // Default case
        llvm::BasicBlock* defaultBB = stmt->defaultCase ? llvm::BasicBlock::Create(*context.llvmContext, "default") : mergeBB;
        if (stmt->defaultCase) {
            currentFunction->insert(currentFunction->end(), defaultBB);
            builder->SetInsertPoint(defaultBB);
            stmt->defaultCase->accept(*this);
            builder->CreateBr(mergeBB);
        }

        // Chain conditions
        llvm::BasicBlock* currentBB = caseBBs[0];
        builder->CreateBr(currentBB);
        for (size_t i = 0; i < caseBBs.size(); ++i) {
            builder->SetInsertPoint(caseBBs[i]);
            builder->CreateCondBr(conditions[i], caseBBs[i], (i + 1 < caseBBs.size()) ? caseBBs[i + 1] : defaultBB);
        }

        currentFunction->insert(currentFunction->end(), mergeBB);
        builder->SetInsertPoint(mergeBB);
    }
}