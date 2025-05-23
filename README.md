# Tocin Compiler Fixes

This document summarizes the changes made to fix the compilation errors in the Tocin compiler.

## Error Fixes

1. **Error Code Definitions**
   - Added missing error code definitions in `error/error_handler.h`:
     - `B001_USE_AFTER_MOVE`
     - `B002_BORROW_CONFLICT`
     - `B003_MUTABILITY_ERROR`
     - `B004_MOVE_BORROWED_VALUE`
     - `P001_NON_EXHAUSTIVE_PATTERNS`

2. **Pattern Matching Fixes**
   - Fixed accessing private fields in `ConstructorPattern`:
     - Changed `pattern->name` to `pattern->getName()`
     - Changed `pattern->patterns` to `pattern->getArguments()`

3. **Null Safety Type System**
   - Added `NullableType` class to `ast/types.h` to enable the nullable type system:
     ```cpp
     class NullableType : public Type {
     public:
         TypePtr baseType;  // The type that can be null
         NullableType(const lexer::Token &token, TypePtr baseType)
             : Type(token), baseType(std::move(baseType)) {}
         std::string toString() const override { return baseType->toString() + "?"; }
     };
     ```
   - Updated the `NullSafetyChecker` methods to use the new `ast::NullableType` class
   - Fixed expression classes in `null_safety.h` to inherit from proper base classes

4. **LLVM IR Generation Fixes**
   - Fixed `CreateMalloc` calls in the dictionary implementation:
     - Updated parameter order to match the LLVM API
   - Updated pointer type access:
     - Replaced `getPointerElementType()` with `getElementType()` to support LLVM's opaque pointers
   - Fixed parameter name access in `generateMethod`:
     - Changed `method->parameters[idx].name.lexeme` to `method->parameters[idx].name`

5. **Fixed Malformed Code**
   - Identified and addressed incomplete code blocks

## Build Instructions

To build the project:

1. Use the MSYS2 environment:
   ```
   cd /c/Users/Afolabi\ Oluwatoisn\ A/Downloads/tocin-compiler/tocin-compiler
   cmake -S . -B build -G "MinGW Makefiles"
   cmake --build build
   ```

2. Alternatively, you can use PowerShell:
   ```powershell
   cd "C:\Users\Afolabi Oluwatoisn A\Downloads\tocin-compiler\tocin-compiler"
   C:\msys64\mingw64\bin\cmake.exe -S . -B build -G "MinGW Makefiles"
   C:\msys64\mingw64\bin\cmake.exe --build build
   ```

Note: Make sure to use the correct generator for your environment. The previous build was using "MinGW Makefiles".
