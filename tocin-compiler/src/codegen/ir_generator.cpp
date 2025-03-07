#include "ir_generator.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <stdexcept>

// Include LLVM PassBuilder headers for optimization passes.
#include "llvm/Passes/PassBuilder.h"

IRGenerator::IRGenerator() {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("TocinModule", *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
    currentSymbolTable = new SymbolTable();
    currentFunction = nullptr;

    // Initialize native target for code generation.
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

void IRGenerator::generate(std::shared_ptr<ast::Statement> ast, const std::string& outputFile) {
    generateGlobalInitialization();
    generateStandardLibraryFunctions();

    // Traverse the AST.
    ast->accept(*this);

    // --- NEW: Run LLVM optimization passes ---
    {
        llvm::PassBuilder passBuilder;
        llvm::LoopAnalysisManager loopAM;
        llvm::FunctionAnalysisManager functionAM;
        llvm::CGSCCAnalysisManager cgsccAM;
        llvm::ModuleAnalysisManager moduleAM;
        passBuilder.registerModuleAnalyses(moduleAM);
        passBuilder.registerCGSCCAnalyses(cgsccAM);
        passBuilder.registerFunctionAnalyses(functionAM);
        passBuilder.registerLoopAnalyses(loopAM);
        passBuilder.crossRegisterProxies(loopAM, functionAM, cgsccAM, moduleAM);

        llvm::ModulePassManager modulePM = passBuilder.buildPerModuleDefaultPipeline(llvm::PassBuilder::O2);
        modulePM.run(*module, moduleAM);
    }
    // --- End of LLVM optimizations ---

    if (!verifyAndWriteOutput(outputFile)) {
        throw std::runtime_error("Failed to verify and write LLVM IR output");
    }
}


// Helper methods
void IRGenerator::beginScope() {
    currentSymbolTable = new SymbolTable(currentSymbolTable);
}

void IRGenerator::endScope() {
    SymbolTable* temp = currentSymbolTable;
    currentSymbolTable = currentSymbolTable->parent;
    delete temp;
}

llvm::Type* IRGenerator::toLLVMType(std::shared_ptr<ast::Type> type) {
    std::string typeName = type->toString();
    if (typeName == "int") {
        return llvm::Type::getInt64Ty(*context);
    }
    else if (typeName == "float64") {
        return llvm::Type::getDoubleTy(*context);
    }
    else if (typeName == "bool") {
        return llvm::Type::getInt1Ty(*context);
    }
    else if (typeName == "string") {
        return llvm::Type::getInt8PtrTy(*context);
    }
    else if (typeName == "None") {
        return llvm::Type::getVoidTy(*context);
    }
    return llvm::Type::getInt64Ty(*context);
}

void IRGenerator::pushValue(llvm::Value* value) {
    valueStack.push(value);
}

llvm::Value* IRGenerator::popValue() {
    if (valueStack.empty()) return nullptr;
    llvm::Value* val = valueStack.top();
    valueStack.pop();
    return val;
}

// Expression visitors
void IRGenerator::visitBinaryExpr(ast::BinaryExpr* expr) {
    expr->left->accept(*this);
    llvm::Value* leftVal = popValue();
    expr->right->accept(*this);
    llvm::Value* rightVal = popValue();

    llvm::Value* result = nullptr;
    switch (expr->op.type) {
    case TokenType::PLUS:
        result = builder->CreateAdd(leftVal, rightVal, "addtmp");
        break;
    case TokenType::MINUS:
        result = builder->CreateSub(leftVal, rightVal, "subtmp");
        break;
    case TokenType::STAR:
        result = builder->CreateMul(leftVal, rightVal, "multmp");
        break;
    case TokenType::SLASH:
        result = builder->CreateSDiv(leftVal, rightVal, "divtmp");
        break;
    default:
        throw std::runtime_error("Unsupported binary operator");
    }
    pushValue(result);
}

void IRGenerator::visitGroupingExpr(ast::GroupingExpr* expr) {
    expr->expression->accept(*this);
}

void IRGenerator::visitLiteralExpr(ast::LiteralExpr* expr) {
    llvm::Value* value = nullptr;
    switch (expr->literalType) {
    case ast::LiteralExpr::LiteralType::INTEGER:
        value = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), std::stoll(expr->value));
        break;
    case ast::LiteralExpr::LiteralType::FLOAT:
        value = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), std::stod(expr->value));
        break;
    case ast::LiteralExpr::LiteralType::STRING:
        value = builder->CreateGlobalStringPtr(expr->value);
        break;
    case ast::LiteralExpr::LiteralType::BOOLEAN:
        value = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), (expr->value == "True") ? 1 : 0);
        break;
    case ast::LiteralExpr::LiteralType::NIL:
        value = llvm::Constant::getNullValue(llvm::Type::getInt64Ty(*context));
        break;
    default:
        throw std::runtime_error("Unsupported literal type");
    }
    pushValue(value);
}

void IRGenerator::visitUnaryExpr(ast::UnaryExpr* expr) {
    expr->right->accept(*this);
    llvm::Value* operand = popValue();
    llvm::Value* result = nullptr;
    switch (expr->op.type) {
    case TokenType::MINUS:
        result = builder->CreateNeg(operand, "negtmp");
        break;
    case TokenType::BANG:
        result = builder->CreateNot(operand, "nottmp");
        break;
    default:
        throw std::runtime_error("Unsupported unary operator");
    }
    pushValue(result);
}

void IRGenerator::visitVariableExpr(ast::VariableExpr* expr) {
    llvm::Value* value = currentSymbolTable->get(expr->name);
    if (!value) throw std::runtime_error("Undefined variable: " + expr->name);
    pushValue(value);
}

void IRGenerator::visitAssignExpr(ast::AssignExpr* expr) {
    expr->value->accept(*this);
    llvm::Value* value = popValue();
    llvm::Value* varPtr = currentSymbolTable->get(expr->name);
    if (!varPtr) throw std::runtime_error("Undefined variable in assignment: " + expr->name);
    builder->CreateStore(value, varPtr);
    pushValue(value);
}

void IRGenerator::visitCallExpr(ast::CallExpr* expr) {
    expr->callee->accept(*this);
    llvm::Value* calleeValue = popValue();

    std::vector<llvm::Value*> args;
    for (auto& arg : expr->arguments) {
        arg->accept(*this);
        args.push_back(popValue());
    }
    llvm::Value* call = builder->CreateCall(calleeValue, args, "calltmp");
    pushValue(call);
}

void IRGenerator::visitGetExpr(ast::GetExpr* expr) {
    throw std::runtime_error("Property get not implemented in IRGenerator");
}

void IRGenerator::visitSetExpr(ast::SetExpr* expr) {
    throw std::runtime_error("Property set not implemented in IRGenerator");
}

void IRGenerator::visitListExpr(ast::ListExpr* expr) {
    std::vector<llvm::Value*> elements;
    for (auto& element : expr->elements) {
        element->accept(*this);
        elements.push_back(popValue());
    }
    llvm::ArrayType* arrayType = llvm::ArrayType::get(llvm::Type::getInt64Ty(*context), elements.size());
    llvm::Constant* arrayConst = llvm::ConstantArray::get(arrayType, elements);
    pushValue(arrayConst);
}

void IRGenerator::visitDictionaryExpr(ast::DictionaryExpr* expr) {
    throw std::runtime_error("Dictionary expressions not implemented in IRGenerator");
}

void IRGenerator::visitLambdaExpr(ast::LambdaExpr* expr) {
    throw std::runtime_error("Lambda expressions not implemented in IRGenerator");
}

// Statement visitors
void IRGenerator::visitExpressionStmt(ast::ExpressionStmt* stmt) {
    stmt->expression->accept(*this);
    popValue();
}

void IRGenerator::visitVariableStmt(ast::VariableStmt* stmt) {
    llvm::Type* varType = toLLVMType(stmt->type);
    llvm::AllocaInst* alloca = builder->CreateAlloca(varType, nullptr, stmt->name);
    if (stmt->initializer) {
        stmt->initializer->accept(*this);
        llvm::Value* initVal = popValue();
        builder->CreateStore(initVal, alloca);
    }
    currentSymbolTable->define(stmt->name, alloca);
}

void IRGenerator::visitBlockStmt(ast::BlockStmt* stmt) {
    beginScope();
    for (auto& s : stmt->statements) {
        s->accept(*this);
    }
    endScope();
}

void IRGenerator::visitIfStmt(ast::IfStmt* stmt) {
    stmt->condition->accept(*this);
    llvm::Value* condVal = popValue();

    llvm::Function* func = builder->GetInsertBlock()->getParent();

    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context, "then", func);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*context, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "ifcont");

    builder->CreateCondBr(condVal, thenBB, elseBB);

    builder->SetInsertPoint(thenBB);
    stmt->thenBranch->accept(*this);
    builder->CreateBr(mergeBB);
    thenBB = builder->GetInsertBlock();

    func->getBasicBlockList().push_back(elseBB);
    builder->SetInsertPoint(elseBB);
    if (stmt->elseBranch)
        stmt->elseBranch->accept(*this);
    builder->CreateBr(mergeBB);
    elseBB = builder->GetInsertBlock();

    func->getBasicBlockList().push_back(mergeBB);
    builder->SetInsertPoint(mergeBB);
}

void IRGenerator::visitWhileStmt(ast::WhileStmt* stmt) {
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(*context, "whilecond", func);
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*context, "whilebody");
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(*context, "whileafter");

    builder->CreateBr(condBB);
    builder->SetInsertPoint(condBB);
    stmt->condition->accept(*this);
    llvm::Value* condVal = popValue();
    builder->CreateCondBr(condVal, loopBB, afterBB);

    func->getBasicBlockList().push_back(loopBB);
    builder->SetInsertPoint(loopBB);
    stmt->body->accept(*this);
    builder->CreateBr(condBB);

    func->getBasicBlockList().push_back(afterBB);
    builder->SetInsertPoint(afterBB);
}

void IRGenerator::visitForStmt(ast::ForStmt* stmt) {
    throw std::runtime_error("For loops not implemented in IRGenerator");
}

void IRGenerator::visitFunctionStmt(ast::FunctionStmt* stmt) {
    std::vector<llvm::Type*> paramTypes;
    for (auto& param : stmt->parameters) {
        paramTypes.push_back(toLLVMType(param.type));
    }
    llvm::Type* retType = toLLVMType(stmt->returnType);
    llvm::FunctionType* funcType = llvm::FunctionType::get(retType, paramTypes, false);
    llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, stmt->name, module.get());
    currentFunction = func;

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(entry);
    beginScope();

    unsigned idx = 0;
    for (auto& arg : func->args()) {
        std::string paramName = stmt->parameters[idx].name;
        arg.setName(paramName);
        llvm::AllocaInst* alloca = builder->CreateAlloca(arg.getType(), nullptr, paramName);
        builder->CreateStore(&arg, alloca);
        currentSymbolTable->define(paramName, alloca);
        idx++;
    }
    stmt->body->accept(*this);
    endScope();
    currentFunction = nullptr;
}

void IRGenerator::visitReturnStmt(ast::ReturnStmt* stmt) {
    llvm::Value* retVal = nullptr;
    if (stmt->value) {
        stmt->value->accept(*this);
        retVal = popValue();
    }
    builder->CreateRet(retVal);
}

void IRGenerator::visitClassStmt(ast::ClassStmt* stmt) {
    throw std::runtime_error("Class statements not implemented in IRGenerator");
}

void IRGenerator::visitImportStmt(ast::ImportStmt* stmt) {
    // Imports are handled at a higher level.
}

void IRGenerator::visitMatchStmt(ast::MatchStmt* stmt) {
    throw std::runtime_error("Match statements not implemented in IRGenerator");
}

void IRGenerator::generateGlobalInitialization() {
    // Insert global variable initialization here if needed.
}

void IRGenerator::generateStandardLibraryFunctions() {
    // Insert standard library function definitions (like 'print') here if desired.
}

bool IRGenerator::verifyAndWriteOutput(const std::string& outputFile) {
    std::string errStr;
    llvm::raw_string_ostream errStream(errStr);
    if (llvm::verifyModule(*module, &errStream)) {
        errStream.flush();
        std::cerr << "Module verification error: " << errStr << std::endl;
        return false;
    }
    std::error_code EC;
    llvm::raw_fd_ostream dest(outputFile, EC, llvm::sys::fs::OF_None);
    if (EC) {
        std::cerr << "Could not open file: " << EC.message() << std::endl;
        return false;
    }
    module->print(dest, nullptr);
    return true;
}
