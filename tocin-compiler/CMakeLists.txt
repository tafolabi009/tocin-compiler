cmake_minimum_required(VERSION 3.10)
project(tocin-compiler)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Locate LLVM using its CMake configuration.
# The LLVM_DIR variable should be set (typically by vcpkg) to point to the LLVM config files.
find_package(LLVM REQUIRED CONFIG)
if(LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
else()
    message(FATAL_ERROR "LLVM not found. Make sure LLVM is installed and LLVM_DIR is set.")
endif()

# Use LLVM's compile definitions and include directories.
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Collect all .cpp source files in the "src" directory.
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable target.
add_executable(tocin-compiler ${SOURCES})

# Set C++ standard to C++20 if supported.
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET tocin-compiler PROPERTY CXX_STANDARD 20)
endif()

# Link LLVM libraries to your target.
target_link_libraries(tocin-compiler ${LLVM_LIBRARIES})

# TODO: Add tests and install targets if needed.
