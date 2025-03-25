#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <filesystem>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "type/type_checker.h"
#include "lexer/token.h"
#include "ir/ir_generator.h"
#include "compiler/compilation_context.h"
#include "error/error_handler.h"

// Print help and usage instructions.
void printUsage() {
    std::cout << "Tocin Compiler v0.3.1\n";
    std::cout << "Usage:\n";
    std::cout << "  tocin-compiler [options] <file.to>\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help        Show this help message.\n";
    std::cout << "  -O <level>        Set optimization level (default: 0, max: 3).\n";
    std::cout << "  -g                Enable debug information.\n";
    std::cout << "  -o <output file>  Specify output file for generated LLVM IR (default: <input>.ll).\n";
    std::cout << "  -r                Enter REPL mode.\n";
    std::cout << "  -v                Enable verbose output.\n";
    std::cout << "  --dump-ast        Dump the parsed AST to stdout or a file (append .ast to output).\n";
    std::cout << "  --time            Show detailed timing breakdown for compilation phases.\n";
    std::cout << "  --history         Enable REPL history (saved to ~/.tocin_history).\n";
    std::cout << "\nExamples:\n";
    std::cout << "  tocin-compiler myprogram.to\n";
    std::cout << "  tocin-compiler -O 2 -g -o output.ll myprogram.to\n";
    std::cout << "  tocin-compiler -r --history\n";
}

// Helper to dump AST (simple string representation, fallback if toString is missing)
std::string dumpAST(const std::shared_ptr<ast::Statement>& stmt) {
    if (!stmt) return "null";
    std::ostringstream oss;
    if (auto exprStmt = std::dynamic_pointer_cast<ast::ExpressionStmt>(stmt)) {
        oss << "ExpressionStmt(expr)";
    }
    else if (auto varStmt = std::dynamic_pointer_cast<ast::VariableStmt>(stmt)) {
        oss << "VariableStmt(" << varStmt->name.value << ")";
    }
    else {
        oss << "Statement(unknown)";
    }
    return oss.str();
}

// REPL Mode (Interactive Interpreter)
void launchREPL(bool useHistory, bool verbose) {
    std::cout << "Tocin REPL (type 'exit' to quit)\n";
    std::string input;
    std::vector<std::string> history;
    std::string historyFile = std::string(getenv("HOME")) + "/.tocin_history";

    if (useHistory) {
        std::ifstream histIn(historyFile);
        std::string line;
        while (std::getline(histIn, line)) {
            history.push_back(line);
        }
        histIn.close();
    }

    while (true) {
        std::cout << ">>> ";
        std::getline(std::cin, input);

        if (input == "exit") break;
        if (input.empty()) continue;

        if (useHistory) {
            history.push_back(input);
            std::ofstream histOut(historyFile, std::ios::app);
            histOut << input << "\n";
            histOut.close();
        }

        try {
            compiler::CompilationContext ctx("<stdin>");
            if (verbose) std::cout << "[INFO] REPL: Initialized context\n";

            Lexer lexer(input, "<stdin>");
            auto tokens = lexer.tokenize();
            if (verbose) std::cout << "[INFO] REPL: Tokenized input\n";

            auto parser = std::make_unique<Parser>(tokens);
            auto ast = parser->parse();
            if (verbose) std::cout << "[INFO] REPL: Parsed AST\n";

            TypeChecker typeChecker;
            typeChecker.check(ast);
            if (verbose) std::cout << "[INFO] REPL: Type checked\n";

            ir::IRGenerator irGen(ctx);
            irGen.generate(ast);
            if (verbose) std::cout << "[INFO] REPL: Generated IR\n";

            if (ctx.errorHandler->hasErrors()) {
                std::cerr << "REPL execution failed due to errors:\n";
                for (const auto& err : ctx.errorHandler->getErrors()) {
                    std::cerr << err.filename << ":" << err.line << ":" << err.column << ": "
                        << (err.severity == error::ErrorSeverity::ERROR ? "Error" : "Warning") << ": "
                        << err.message << "\n";
                }
            }
            else if (verbose) {
                std::cout << "[INFO] REPL: Execution successful\n";
            }
        }
        catch (const std::exception& e) {
            std::cerr << "REPL Error: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string inputFile;
    std::string outputFile;
    int optimizationLevel = 0;
    bool debug = false;
    bool useREPL = false;
    bool verbose = false;
    bool dumpAST = false;
    bool showTiming = false;
    bool useHistory = false;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        }
        else if (arg == "-O") {
            if (i + 1 < argc) {
                try {
                    optimizationLevel = std::stoi(argv[++i]);
                    if (optimizationLevel < 0 || optimizationLevel > 3) {
                        std::cerr << "Warning: Invalid optimization level. Using default (0).\n";
                        optimizationLevel = 0;
                    }
                }
                catch (...) {
                    std::cerr << "Error: Invalid optimization level provided.\n";
                    return 1;
                }
            }
            else {
                std::cerr << "Error: Missing optimization level after -O.\n";
                return 1;
            }
        }
        else if (arg == "-g") {
            debug = true;
        }
        else if (arg == "-o") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            }
            else {
                std::cerr << "Error: Missing output file after -o.\n";
                return 1;
            }
        }
        else if (arg == "-r") {
            useREPL = true;
        }
        else if (arg == "-v") {
            verbose = true;
        }
        else if (arg == "--dump-ast") {
            dumpAST = true;
        }
        else if (arg == "--time") {
            showTiming = true;
        }
        else if (arg == "--history") {
            useHistory = true;
        }
        else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        }
        else {
            inputFile = arg;
        }
    }

    if (useREPL) {
        launchREPL(useHistory, verbose);
        return 0;
    }

    if (inputFile.empty()) {
        std::cerr << "Error: No input file provided.\n";
        return 1;
    }

    if (outputFile.empty()) {
        outputFile = inputFile.substr(0, inputFile.find_last_of('.')) + ".ll";
    }

    std::ifstream file(inputFile);
    if (!file) {
        std::cerr << "Error: Could not open file " << inputFile << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sourceCode = buffer.str();
    file.close();

    if (sourceCode.starts_with("#!")) {
        size_t newline = sourceCode.find('\n');
        if (newline != std::string::npos) {
            sourceCode = sourceCode.substr(newline + 1);
        }
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> lexingTime, parsingTime, typeCheckTime, irGenTime;

    try {
        if (verbose) std::cout << "[INFO] Starting compilation for " << inputFile << "\n";

        compiler::CompilationContext ctx(inputFile);
        if (verbose) std::cout << "[INFO] Compilation context initialized.\n";

        auto lexStart = std::chrono::high_resolution_clock::now();
        Lexer lexer(sourceCode, inputFile);
        auto tokens = lexer.tokenize();
        auto lexEnd = std::chrono::high_resolution_clock::now();
        lexingTime = lexEnd - lexStart;
        if (verbose) std::cout << "[INFO] Lexing completed.\n";

        auto parseStart = std::chrono::high_resolution_clock::now();
        auto parser = std::make_unique<Parser>(tokens);
        auto ast = parser->parse();
        auto parseEnd = std::chrono::high_resolution_clock::now();
        parsingTime = parseEnd - parseStart;
        if (verbose) std::cout << "[INFO] Parsing completed.\n";

        if (dumpAST) {
            std::string astFile = dumpAST ? (outputFile + ".ast") : "";
            std::ostream* out = astFile.empty() ? &std::cout : new std::ofstream(astFile);
            *out << dumpAST(ast) << "\n";
            if (!astFile.empty()) {
                static_cast<std::ofstream*>(out)->close();
                delete out;
                if (verbose) std::cout << "[INFO] AST dumped to " << astFile << "\n";
            }
        }

        auto typeStart = std::chrono::high_resolution_clock::now();
        TypeChecker typeChecker;
        typeChecker.check(ast);
        auto typeEnd = std::chrono::high_resolution_clock::now();
        typeCheckTime = typeEnd - typeStart;
        if (verbose) std::cout << "[INFO] Type checking completed.\n";

        auto irStart = std::chrono::high_resolution_clock::now();
        ir::IRGenerator irGen(ctx);
        irGen.generate(ast);
        auto irEnd = std::chrono::high_resolution_clock::now();
        irGenTime = irEnd - irStart;
        if (verbose) std::cout << "[INFO] IR Generation completed.\n";

        std::ofstream outFile(outputFile);
        if (!outFile) {
            std::cerr << "Error: Could not open output file " << outputFile << "\n";
            return 1;
        }
        ctx.module->print(llvm::raw_os_ostream(outFile), nullptr);
        outFile.close();
        if (verbose) std::cout << "[INFO] LLVM IR written to " << outputFile << "\n";

        if (ctx.errorHandler->hasErrors()) {
            std::cerr << "Compilation failed due to errors:\n";
            for (const auto& err : ctx.errorHandler->getErrors()) {
                std::cerr << err.filename << ":" << err.line << ":" << err.column << ": "
                    << (err.severity == error::ErrorSeverity::ERROR ? "Error" : "Warning") << ": "
                    << err.message << "\n";
            }
            return 1;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Compilation Error: " << e.what() << std::endl;
        return 1;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;

    if (showTiming) {
        std::cout << "Compilation Timing Breakdown:\n";
        std::cout << "  Lexing: " << lexingTime.count() << " seconds\n";
        std::cout << "  Parsing: " << parsingTime.count() << " seconds\n";
        std::cout << "  Type Checking: " << typeCheckTime.count() << " seconds\n";
        std::cout << "  IR Generation: " << irGenTime.count() << " seconds\n";
        std::cout << "  Total: " << elapsed.count() << " seconds\n";
    }
    else {
        std::cout << "Compilation completed in " << elapsed.count() << " seconds.\n";
    }

    return 0;
}