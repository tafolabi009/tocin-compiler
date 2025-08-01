#include "macro_system.h"
#include "../ast/ast.h"
#include "../ast/visitor.h"
#include <sstream>
#include <algorithm>
#include <random>

namespace compiler {

// FunctionMacro implementation
ast::StmtPtr FunctionMacro::expand(const MacroContext& context, error::ErrorHandler& errorHandler) {
    // Check argument count
    if (!variadic && context.arguments.size() != parameters.size()) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "Macro '" + name + "' expects " + std::to_string(parameters.size()) + 
                               " arguments, got " + std::to_string(context.arguments.size()));
        return nullptr;
    }
    
    // Create substitution context
    MacroContext subContext = context;
    for (size_t i = 0; i < std::min(parameters.size(), context.arguments.size()); ++i) {
        subContext.capturedVars[parameters[i]] = context.arguments[i];
    }
    
    // Substitute arguments in the body
    MacroSystem macroSystem;
    return macroSystem.substituteMacroArguments(body, subContext);
}

// MacroSystem implementation
void MacroSystem::registerMacro(std::unique_ptr<MacroDefinition> macro) {
    if (macro) {
        macros[macro->getName()] = std::move(macro);
    }
}

ast::StmtPtr MacroSystem::expandMacro(const std::string& macroName, const MacroContext& context, 
                                     error::ErrorHandler& errorHandler) {
    auto it = macros.find(macroName);
    if (it == macros.end()) {
        errorHandler.reportError(error::ErrorCode::T003_UNDEFINED_FUNCTION,
                               "Undefined macro: " + macroName);
        return nullptr;
    }
    
    // Check expansion depth to prevent infinite recursion
    if (context.expansionDepth > maxExpansionDepth) {
        errorHandler.reportError(error::ErrorCode::C001_UNIMPLEMENTED_FEATURE,
                               "Macro expansion depth limit exceeded");
        return nullptr;
    }
    
    MacroContext newContext = context;
    newContext.expansionDepth++;
    
    return it->second->expand(newContext, errorHandler);
}

bool MacroSystem::hasMacro(const std::string& name) const {
    return macros.find(name) != macros.end();
}

MacroDefinition* MacroSystem::getMacro(const std::string& name) {
    auto it = macros.find(name);
    return it != macros.end() ? it->second.get() : nullptr;
}

std::unique_ptr<MacroDefinition> MacroSystem::parseMacroDefinition(ast::StmtPtr stmt, 
                                                                  error::ErrorHandler& errorHandler) {
    // This would parse macro definitions from AST
    // For now, return nullptr as placeholder
    return nullptr;
}

ast::StmtPtr MacroSystem::processMacros(ast::StmtPtr stmt, error::ErrorHandler& errorHandler) {
    if (!stmt) return nullptr;
    
    // Create a visitor to process macros in statements
    class MacroVisitor : public ast::Visitor {
    private:
        MacroSystem& macroSystem;
        error::ErrorHandler& errorHandler;
        
    public:
        MacroVisitor(MacroSystem& ms, error::ErrorHandler& eh) 
            : macroSystem(ms), errorHandler(eh) {}
        
        void visitExpressionStmt(ast::ExpressionStmt* stmt) override {
            if (stmt->expression) {
                stmt->expression = macroSystem.processMacros(stmt->expression, errorHandler);
            }
        }
        
        void visitBlockStmt(ast::BlockStmt* stmt) override {
            for (auto& statement : stmt->statements) {
                if (statement) {
                    statement = macroSystem.processMacros(statement, errorHandler);
                }
            }
        }
        
        // Add other visitor methods as needed
        void visitVariableStmt(ast::VariableStmt* stmt) override {}
        void visitFunctionStmt(ast::FunctionStmt* stmt) override {}
        void visitReturnStmt(ast::ReturnStmt* stmt) override {}
        void visitClassStmt(ast::ClassStmt* stmt) override {}
        void visitIfStmt(ast::IfStmt* stmt) override {}
        void visitWhileStmt(ast::WhileStmt* stmt) override {}
        void visitForStmt(ast::ForStmt* stmt) override {}
        void visitMatchStmt(ast::MatchStmt* stmt) override {}
        void visitImportStmt(ast::ImportStmt* stmt) override {}
        void visitExportStmt(ast::ExportStmt* stmt) override {}
        void visitModuleStmt(ast::ModuleStmt* stmt) override {}
        void visitBinaryExpr(ast::BinaryExpr* expr) override {}
        void visitGroupingExpr(ast::GroupingExpr* expr) override {}
        void visitLiteralExpr(ast::LiteralExpr* expr) override {}
        void visitUnaryExpr(ast::UnaryExpr* expr) override {}
        void visitVariableExpr(ast::VariableExpr* expr) override {}
        void visitAssignExpr(ast::AssignExpr* expr) override {}
        void visitCallExpr(ast::CallExpr* expr) override {}
        void visitGetExpr(ast::GetExpr* expr) override {}
        void visitSetExpr(ast::SetExpr* expr) override {}
        void visitListExpr(ast::ListExpr* expr) override {}
        void visitDictionaryExpr(ast::DictionaryExpr* expr) override {}
        void visitLambdaExpr(ast::LambdaExpr* expr) override {}
        void visitAwaitExpr(ast::AwaitExpr* expr) override {}
        void visitNewExpr(ast::NewExpr* expr) override {}
        void visitDeleteExpr(ast::DeleteExpr* expr) override {}
        void visitStringInterpolationExpr(ast::StringInterpolationExpr* expr) override {}
        void visitArrayLiteralExpr(ast::ArrayLiteralExpr* expr) override {}
        void visitMoveExpr(void* expr) override {}
        void visitGoExpr(void* expr) override {}
        void visitRuntimeChannelSendExpr(void* expr) override {}
        void visitRuntimeChannelReceiveExpr(void* expr) override {}
        void visitRuntimeSelectStmt(void* stmt) override {}
        void visitChannelSendExpr(ast::ChannelSendExpr* expr) override {}
        void visitChannelReceiveExpr(ast::ChannelReceiveExpr* expr) override {}
        void visitSelectStmt(ast::SelectStmt* stmt) override {}
        void visitGoStmt(ast::GoStmt* stmt) override {}
        void visitTraitStmt(ast::TraitStmt* stmt) override {}
        void visitImplStmt(ast::ImplStmt* stmt) override {}
    };
    
    MacroVisitor visitor(*this, errorHandler);
    stmt->accept(visitor);
    return stmt;
}

ast::ExprPtr MacroSystem::processMacros(ast::ExprPtr expr, error::ErrorHandler& errorHandler) {
    if (!expr) return nullptr;
    
    // Process macros in expressions
    // This would traverse the expression tree and expand macros
    return expr;
}

void MacroSystem::registerBuiltinMacros() {
    // Register built-in macros
    registerMacro(std::make_unique<ProceduralMacro>("debug", builtin_macros::debugMacro));
    registerMacro(std::make_unique<ProceduralMacro>("assert", builtin_macros::assertMacro));
    registerMacro(std::make_unique<ProceduralMacro>("measure", builtin_macros::measureMacro));
    registerMacro(std::make_unique<ProceduralMacro>("repeat", builtin_macros::repeatMacro));
    registerMacro(std::make_unique<ProceduralMacro>("if", builtin_macros::ifMacro));
    registerMacro(std::make_unique<ProceduralMacro>("match", builtin_macros::matchMacro));
    registerMacro(std::make_unique<ProceduralMacro>("for", builtin_macros::forMacro));
    registerMacro(std::make_unique<ProceduralMacro>("let", builtin_macros::letMacro));
    registerMacro(std::make_unique<ProceduralMacro>("try", builtin_macros::tryMacro));
    registerMacro(std::make_unique<ProceduralMacro>("log", builtin_macros::logMacro));
    registerMacro(std::make_unique<ProceduralMacro>("profile", builtin_macros::profileMacro));
}

ast::StmtPtr MacroSystem::substituteMacroArguments(ast::StmtPtr stmt, const MacroContext& context) {
    // This would substitute macro arguments in statements
    // For now, return the original statement
    return stmt;
}

ast::ExprPtr MacroSystem::substituteMacroArguments(ast::ExprPtr expr, const MacroContext& context) {
    // This would substitute macro arguments in expressions
    // For now, return the original expression
    return expr;
}

std::string MacroSystem::generateUniqueIdentifier(const std::string& base) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    
    return base + "_" + std::to_string(dis(gen));
}

// Built-in macro implementations
namespace builtin_macros {

ast::StmtPtr debugMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    // Create debug statement
    std::string debugInfo = "Debug: " + context.macroName;
    if (!context.arguments.empty()) {
        debugInfo += " with " + std::to_string(context.arguments.size()) + " arguments";
    }
    
    // Create a print statement for debugging
    auto debugToken = lexer::Token(lexer::TokenType::STRING, debugInfo, "", 0, 0);
    auto debugExpr = std::make_shared<ast::LiteralExpr>(debugToken);
    auto debugCall = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, "print", "", 0, 0),
        debugExpr,
        std::vector<ast::ExprPtr>{}
    );
    
    return std::make_shared<ast::ExpressionStmt>(
        lexer::Token(lexer::TokenType::SEMICOLON, ";", "", 0, 0),
        debugCall
    );
}

ast::StmtPtr assertMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    if (context.arguments.empty()) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "assert macro requires at least one argument");
        return nullptr;
    }
    
    // Create assert statement
    auto condition = context.arguments[0];
    std::string message = "Assertion failed";
    
    if (context.arguments.size() > 1) {
        // Second argument is the error message
        if (auto literal = std::dynamic_pointer_cast<ast::LiteralExpr>(context.arguments[1])) {
            message = literal->value;
        }
    }
    
    // Create if statement that throws on assertion failure
    auto messageToken = lexer::Token(lexer::TokenType::STRING, message, "", 0, 0);
    auto messageExpr = std::make_shared<ast::LiteralExpr>(messageToken);
    auto throwCall = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, "throw", "", 0, 0),
        messageExpr,
        std::vector<ast::ExprPtr>{}
    );
    
    auto throwStmt = std::make_shared<ast::ExpressionStmt>(
        lexer::Token(lexer::TokenType::SEMICOLON, ";", "", 0, 0),
        throwCall
    );
    
    std::vector<ast::StmtPtr> thenBody{throwStmt};
    auto thenBlock = std::make_shared<ast::BlockStmt>(
        lexer::Token(lexer::TokenType::LEFT_BRACE, "{", "", 0, 0),
        thenBody
    );
    
    return std::make_shared<ast::IfStmt>(
        lexer::Token(lexer::TokenType::IF, "if", "", 0, 0),
        condition,
        thenBlock,
        nullptr
    );
}

ast::StmtPtr measureMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    if (context.arguments.empty()) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "measure macro requires at least one argument");
        return nullptr;
    }
    
    // Create timing measurement code
    auto startTime = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, "Date.now", "", 0, 0),
        nullptr,
        std::vector<ast::ExprPtr>{}
    );
    
    auto startVar = std::make_shared<ast::VariableStmt>(
        lexer::Token(lexer::TokenType::LET, "let", "", 0, 0),
        "start_time",
        nullptr,
        startTime
    );
    
    // Execute the measured code
    auto measuredCode = context.arguments[0];
    auto measuredStmt = std::dynamic_pointer_cast<ast::Statement>(measuredCode);
    if (!measuredStmt) {
        measuredStmt = std::make_shared<ast::ExpressionStmt>(
            lexer::Token(lexer::TokenType::SEMICOLON, ";", "", 0, 0),
            measuredCode
        );
    }
    
    // Calculate elapsed time
    auto endTime = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, "Date.now", "", 0, 0),
        nullptr,
        std::vector<ast::ExprPtr>{}
    );
    
    auto elapsedExpr = std::make_shared<ast::BinaryExpr>(
        lexer::Token(lexer::TokenType::MINUS, "-", "", 0, 0),
        endTime,
        std::make_shared<ast::VariableExpr>(
            lexer::Token(lexer::TokenType::IDENTIFIER, "start_time", "", 0, 0)
        )
    );
    
    auto elapsedVar = std::make_shared<ast::VariableStmt>(
        lexer::Token(lexer::TokenType::LET, "let", "", 0, 0),
        "elapsed",
        nullptr,
        elapsedExpr
    );
    
    // Print the result
    auto printCall = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, "print", "", 0, 0),
        std::make_shared<ast::BinaryExpr>(
            lexer::Token(lexer::TokenType::PLUS, "+", "", 0, 0),
            std::make_shared<ast::LiteralExpr>(
                lexer::Token(lexer::TokenType::STRING, "Execution time: ", "", 0, 0)
            ),
            std::make_shared<ast::VariableExpr>(
                lexer::Token(lexer::TokenType::IDENTIFIER, "elapsed", "", 0, 0)
            )
        ),
        std::vector<ast::ExprPtr>{}
    );
    
    auto printStmt = std::make_shared<ast::ExpressionStmt>(
        lexer::Token(lexer::TokenType::SEMICOLON, ";", "", 0, 0),
        printCall
    );
    
    // Create block with all statements
    std::vector<ast::StmtPtr> blockStmts{startVar, measuredStmt, elapsedVar, printStmt};
    return std::make_shared<ast::BlockStmt>(
        lexer::Token(lexer::TokenType::LEFT_BRACE, "{", "", 0, 0),
        blockStmts
    );
}

ast::StmtPtr repeatMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    if (context.arguments.size() < 2) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "repeat macro requires count and body arguments");
        return nullptr;
    }
    
    // Create for loop
    auto count = context.arguments[0];
    auto body = context.arguments[1];
    
    auto forStmt = std::make_shared<ast::ForStmt>(
        lexer::Token(lexer::TokenType::FOR, "for", "", 0, 0),
        std::make_shared<ast::VariableExpr>(
            lexer::Token(lexer::TokenType::IDENTIFIER, "i", "", 0, 0)
        ),
        count,
        std::dynamic_pointer_cast<ast::Statement>(body)
    );
    
    return forStmt;
}

ast::StmtPtr ifMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    if (context.arguments.size() < 2) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "if macro requires condition and then body");
        return nullptr;
    }
    
    auto condition = context.arguments[0];
    auto thenBody = context.arguments[1];
    ast::StmtPtr elseBody = nullptr;
    
    if (context.arguments.size() > 2) {
        elseBody = std::dynamic_pointer_cast<ast::Statement>(context.arguments[2]);
    }
    
    return std::make_shared<ast::IfStmt>(
        lexer::Token(lexer::TokenType::IF, "if", "", 0, 0),
        condition,
        std::dynamic_pointer_cast<ast::Statement>(thenBody),
        elseBody
    );
}

ast::StmtPtr matchMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    if (context.arguments.empty()) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "match macro requires expression argument");
        return nullptr;
    }
    
    auto expression = context.arguments[0];
    
    // Create match statement (simplified)
    return std::make_shared<ast::MatchStmt>(
        lexer::Token(lexer::TokenType::MATCH, "match", "", 0, 0),
        expression,
        std::vector<ast::MatchCase>{}
    );
}

ast::StmtPtr forMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    if (context.arguments.size() < 2) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "for macro requires iterator and body");
        return nullptr;
    }
    
    auto iterator = context.arguments[0];
    auto body = context.arguments[1];
    
    return std::make_shared<ast::ForStmt>(
        lexer::Token(lexer::TokenType::FOR, "for", "", 0, 0),
        nullptr,
        iterator,
        std::dynamic_pointer_cast<ast::Statement>(body)
    );
}

ast::StmtPtr letMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    if (context.arguments.size() < 2) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "let macro requires name and value");
        return nullptr;
    }
    
    auto name = context.arguments[0];
    auto value = context.arguments[1];
    
    std::string varName = "temp";
    if (auto literal = std::dynamic_pointer_cast<ast::LiteralExpr>(name)) {
        varName = literal->value;
    }
    
    return std::make_shared<ast::VariableStmt>(
        lexer::Token(lexer::TokenType::LET, "let", "", 0, 0),
        varName,
        nullptr,
        value
    );
}

ast::StmtPtr tryMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    if (context.arguments.empty()) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "try macro requires body");
        return nullptr;
    }
    
    auto body = context.arguments[0];
    
    // Create try-catch block (simplified)
    std::vector<ast::StmtPtr> tryBody{std::dynamic_pointer_cast<ast::Statement>(body)};
    auto tryBlock = std::make_shared<ast::BlockStmt>(
        lexer::Token(lexer::TokenType::LEFT_BRACE, "{", "", 0, 0),
        tryBody
    );
    
    // For now, just return the try block
    return tryBlock;
}

ast::StmtPtr logMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    if (context.arguments.empty()) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "log macro requires message");
        return nullptr;
    }
    
    auto message = context.arguments[0];
    
    // Create log statement with timestamp
    auto timestamp = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, "Date.now", "", 0, 0),
        nullptr,
        std::vector<ast::ExprPtr>{}
    );
    
    auto logMessage = std::make_shared<ast::BinaryExpr>(
        lexer::Token(lexer::TokenType::PLUS, "+", "", 0, 0),
        std::make_shared<ast::LiteralExpr>(
            lexer::Token(lexer::TokenType::STRING, "[LOG] ", "", 0, 0)
        ),
        message
    );
    
    auto printCall = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, "print", "", 0, 0),
        logMessage,
        std::vector<ast::ExprPtr>{}
    );
    
    return std::make_shared<ast::ExpressionStmt>(
        lexer::Token(lexer::TokenType::SEMICOLON, ";", "", 0, 0),
        printCall
    );
}

ast::StmtPtr profileMacro(const MacroContext& context, error::ErrorHandler& errorHandler) {
    if (context.arguments.empty()) {
        errorHandler.reportError(error::ErrorCode::S001_UNEXPECTED_TOKEN,
                               "profile macro requires function name");
        return nullptr;
    }
    
    auto functionName = context.arguments[0];
    
    // Create profiling wrapper
    std::string name = "function";
    if (auto literal = std::dynamic_pointer_cast<ast::LiteralExpr>(functionName)) {
        name = literal->value;
    }
    
    // Create profiling code (simplified)
    auto startTime = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, "performance.now", "", 0, 0),
        nullptr,
        std::vector<ast::ExprPtr>{}
    );
    
    auto startVar = std::make_shared<ast::VariableStmt>(
        lexer::Token(lexer::TokenType::LET, "let", "", 0, 0),
        "start_time",
        nullptr,
        startTime
    );
    
    // Call the function
    auto funcCall = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, name, "", 0, 0),
        nullptr,
        std::vector<ast::ExprPtr>{}
    );
    
    auto callStmt = std::make_shared<ast::ExpressionStmt>(
        lexer::Token(lexer::TokenType::SEMICOLON, ";", "", 0, 0),
        funcCall
    );
    
    // Calculate duration
    auto endTime = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, "performance.now", "", 0, 0),
        nullptr,
        std::vector<ast::ExprPtr>{}
    );
    
    auto duration = std::make_shared<ast::BinaryExpr>(
        lexer::Token(lexer::TokenType::MINUS, "-", "", 0, 0),
        endTime,
        std::make_shared<ast::VariableExpr>(
            lexer::Token(lexer::TokenType::IDENTIFIER, "start_time", "", 0, 0)
        )
    );
    
    auto durationVar = std::make_shared<ast::VariableStmt>(
        lexer::Token(lexer::TokenType::LET, "let", "", 0, 0),
        "duration",
        nullptr,
        duration
    );
    
    // Print result
    auto printCall = std::make_shared<ast::CallExpr>(
        lexer::Token(lexer::TokenType::IDENTIFIER, "print", "", 0, 0),
        std::make_shared<ast::BinaryExpr>(
            lexer::Token(lexer::TokenType::PLUS, "+", "", 0, 0),
            std::make_shared<ast::LiteralExpr>(
                lexer::Token(lexer::TokenType::STRING, "Function " + name + " took: ", "", 0, 0)
            ),
            std::make_shared<ast::VariableExpr>(
                lexer::Token(lexer::TokenType::IDENTIFIER, "duration", "", 0, 0)
            )
        ),
        std::vector<ast::ExprPtr>{}
    );
    
    auto printStmt = std::make_shared<ast::ExpressionStmt>(
        lexer::Token(lexer::TokenType::SEMICOLON, ";", "", 0, 0),
        printCall
    );
    
    std::vector<ast::StmtPtr> blockStmts{startVar, callStmt, durationVar, printStmt};
    return std::make_shared<ast::BlockStmt>(
        lexer::Token(lexer::TokenType::LEFT_BRACE, "{", "", 0, 0),
        blockStmts
    );
}

} // namespace builtin_macros

} // namespace compiler 