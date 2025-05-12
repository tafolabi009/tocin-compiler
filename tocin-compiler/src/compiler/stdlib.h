#ifndef STDLIB_H
#define STDLIB_H

#include "ffi/ffi_cpp.h"

namespace compiler {

    class StdLib {
    public:
        static void registerFunctions(ffi::CppFFI& ffi);
    };

} // namespace compiler

#endif // STDLIB_H