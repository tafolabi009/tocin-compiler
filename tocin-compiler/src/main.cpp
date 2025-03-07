#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "type/type_checker.h"
#include "codegen/ir_generator.h"
#include "compiler/compilation_context.h"
#include "error/error_handler.h"

// Print help and usage instructions.
void printUsage() {
    std::cout << "Tocin Compiler v0.2.1\n";
    std::cout << "Usage:\n";
    std::cout << "  tocin [options] <file.to>\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help        Show this help message.\n";
    std::cout << "  -O <level>        Set optimization level (default: 0).\n";
    std::cout << "  -g                Enable debug information.\n";
    std::cout << "\nExamples:\n";
    std::cout << "  tocin myprogram.to         Compile and run 'myprogram.to'.\n";
    std::cout << "  tocin -O 2 -g myprogram.to  Compile with optimization and debug info.\n";
    std::cout << "  tocin                      Launch interactive mode (REPL).\n";
}

// REPL mode: read-eval-print loop.
void startREPL(CompilationContext& context) {
    std::string line;
    std::cout << "Entering Tocin REPL mode. Type ':quit' to exit.\n";
    while (true) {
        std::cout << "tocin> ";
        std::getline(std::cin, line);
        if (line == ":quit") break;
        if (line.empty()) continue;

        // Set the current line as the source code.
        context.sourceCode = line;

        // Lexing
        auto lexer = std::make_unique<Lexer>(context.sourceCode, "REPL");
        auto tokens = lexer->tokenize();

        // Parsing
        auto parser = std::make_unique<Parser>(tokens);
        auto ast = parser->parse();
        if (!ast) {
            std::cerr << "Parsing error. Please check your syntax.\n";
            continue;
        }

        // Type Checking
        auto typeChecker = std::make_unique<TypeChecker>();
        typeChecker->check(ast);
        if (context.hasErrors()) {
            std::cerr << "Type checking error.\n";
            continue;
        }

        // IR Generation (or you might interpret/JIT compile in a future version)
        auto irGenerator = std::make_unique<IRGenerator>();
        std::string tempOutput = "repl_output.ll";
        try {
            irGenerator->generate(ast, tempOutput);
            std::ifstream irFile(tempOutput);
            std::stringstream irContent;
            irContent << irFile.rdbuf();
            std::cout << "Generated IR:\n" << irContent.str() << "\n";
        }
        catch (const std::exception& e) {
            std::cerr << "IR generation error: " << e.what() << "\n";
        }
    }
}

int main(int argc, char** argv) {
    std::string inputFile;
    int optimizationLevel = 0;
    bool debugInfo = false;

    // Parse command-line arguments.
    if (argc == 1) {
        // No arguments: REPL mode.
    }
    else {
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-h" || arg == "--help") {
                printUsage();
                return 0;
            }
            else if (arg == "-O" && i + 1 < argc) {
                optimizationLevel = std::stoi(argv[++i]);
            }
            else if (arg == "-g") {
                debugInfo = true;
            }
            else if (arg[0] != '-') {
                inputFile = arg;
            }
        }
    }

    // Create a compilation context; if no input file, use "interactive" as placeholder.
    CompilationContext context(inputFile.empty() ? "interactive" : inputFile);
    context.optimizationLevel = optimizationLevel;
    if (debugInfo)
        context.enableDebugInfo();

    // REPL mode if no file provided.
    if (inputFile.empty()) {
        startREPL(context);
    }
    else {
        // Read the entire source file.
        std::ifstream inFile(inputFile);
        if (!inFile.is_open()) {
            std::cerr << "Error: Cannot open file " << inputFile << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << inFile.rdbuf();
        context.sourceCode = buffer.str();
        inFile.close();

        // Lexical Analysis
        auto lexer = std::make_unique<Lexer>(context.sourceCode, inputFile);
        auto tokens = lexer->tokenize();

        // Parsing
        auto parser = std::make_unique<Parser>(tokens);
        auto ast = parser->parse();
        if (!ast) {
            std::cerr << "Parsing failed.\n";
            return 1;
        }

        // Type Checking
        auto typeChecker = std::make_unique<TypeChecker>();
        typeChecker->check(ast);
        if (context.hasErrors()) {
            std::cerr << "Type checking failed.\n";
            return 1;
        }

        // IR Generation: Generate LLVM IR output.
        auto irGenerator = std::make_unique<IRGenerator>();
        std::string outputFile = inputFile.substr(0, inputFile.find_last_of('.')) + ".ll";
        try {
            irGenerator->generate(ast, outputFile);
            std::cout << "Compilation successful. Output written to " << outputFile << "\n";
        }
        catch (const std::exception& e) {
            std::cerr << "IR generation error: " << e.what() << "\n";
            return 1;
        }
    }
    return 0;
}
