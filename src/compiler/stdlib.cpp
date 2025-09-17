#include "stdlib.h"
#include <cmath>
#include <iostream>
#include <ostream>

namespace tocin {
namespace compiler {

    void StdLib::registerFunctions(ffi::CppFFI& /*ffi*/) {
        // Temporarily disabled until CppFFIImpl exposes a lambda-based registration API
    }

} // namespace compiler
} // namespace tocin