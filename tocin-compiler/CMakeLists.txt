cmake_minimum_required(VERSION 3.10)
project(tocin-compiler)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# -------------------------
# LLVM Integration
# -------------------------
# Set LLVM_DIR to the MSYS2 LLVM configuration folder.
set(LLVM_DIR "C:/msys64/mingw64/lib/cmake/llvm")

find_package(LLVM REQUIRED CONFIG)
if(LLVM_FOUND)
  message(STATUS "Found LLVM version ${LLVM_VERSION}")
  message(STATUS "Using LLVMConfig.cmake from: ${LLVM_DIR}")
else()
  message(FATAL_ERROR "LLVM not found. Please ensure LLVM is installed and LLVM_DIR is set correctly.")
endif()

include_directories("C:/msys64/mingw64/include")

include_directories("C:/msys64/mingw64/include/python3.12")
link_directories("C:/msys64/mingw64/lib")


# Use LLVM's compile definitions and include directories.
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Find the Python header files.
find_path(PYTHON_INCLUDE_DIR NAMES Python.h
  HINTS "C:/msys64/mingw64/include/python3.12"
)

# Find the PYTHON import library.
find_library(PYTHON_LIBRARY NAMES libpython3.12.dll.a libpython3.dll.a
  HINTS "C:/msys64/mingw64/lib"
)

if(PYTHON_INCLUDE_DIR AND PYTHON_LIBRARY)
  message(STATUS "Found PYTHON headers in: ${V8_INCLUDE_DIR}")
  message(STATUS "Found PYTHON library: ${V8_LIBRARY}")
  include_directories(${V8_INCLUDE_DIR})
  set(EXTRA_LIBS ${V8_LIBRARY})
else()
  message(WARNING "Python not found. Python-related functionality will be disabled.")
  set(EXTRA_LIBS "")
endif()


# -------------------------
# V8 Integration
# -------------------------
# Find V8 header files.
find_path(V8_INCLUDE_DIR v8.h
  HINTS "C:/msys64/mingw64/include"
)

# Find the V8 import library (typically named libv8.dll.a).
find_library(V8_LIBRARY NAMES v8.dll.a libv8.dll.a v8
  HINTS "C:/msys64/mingw64/lib"
)

if(V8_INCLUDE_DIR AND V8_LIBRARY)
  message(STATUS "Found V8 headers in: ${V8_INCLUDE_DIR}")
  message(STATUS "Found V8 library: ${V8_LIBRARY}")
  include_directories(${V8_INCLUDE_DIR})
  set(EXTRA_LIBS ${V8_LIBRARY})
else()
  message(WARNING "V8 not found. V8-related functionality will be disabled.")
  set(EXTRA_LIBS "")
endif()

# -------------------------
# FFI (libffi) Integration - Manual Setup
# -------------------------
set(FFI_INCLUDE_DIR "C:/msys64/mingw64/include/libffi")
set(FFI_LIBRARY "C:/msys64/mingw64/lib/libffi.dll.a")

include_directories(${FFI_INCLUDE_DIR})
set(DEP_LIBS ${FFI_LIBRARY})


# -------------------------
# ZLIB Integration (Manual Setup)
# -------------------------
set(ZLIB_INCLUDE_DIR "C:/msys64/mingw64/include")
set(ZLIB_LIBRARY "C:/msys64/mingw64/lib/libz.dll.a")
include_directories(${ZLIB_INCLUDE_DIR})
set(DEP_LIBS ${DEP_LIBS} ${ZLIB_LIBRARY})

# -------------------------
# zstd Integration (Manual Setup)
# -------------------------
set(zstd_INCLUDE_DIR "C:/msys64/mingw64/include")
set(zstd_LIBRARY "C:/msys64/mingw64/lib/libzstd.dll.a")
include_directories(${zstd_INCLUDE_DIR})
set(DEP_LIBS ${DEP_LIBS} ${zstd_LIBRARY})

# -------------------------
# LibXml2 Integration (Manual Setup)
# -------------------------
set(LibXml2_INCLUDE_DIR "C:/msys64/mingw64/include/libxml2")
set(LibXml2_LIBRARY "C:/msys64/mingw64/lib/libxml2.dll.a")
include_directories(${LibXml2_INCLUDE_DIR})
set(DEP_LIBS ${DEP_LIBS} ${LibXml2_LIBRARY})



# -------------------------
# Source Files
# -------------------------
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable target.
add_executable(tocin-compiler ${SOURCES})

# Set C++ standard to C++20 if supported.
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET tocin-compiler PROPERTY CXX_STANDARD 20)
endif()

# -------------------------
# Linking Libraries
# -------------------------
target_link_libraries(tocin-compiler ${LLVM_LIBRARIES} ${EXTRA_LIBS})
target_link_libraries(tocin-compiler ${LLVM_LIBRARIES} ${EXTRA_LIBS} ${DEP_LIBS})
target_link_libraries(tocin-compiler ${LLVM_LIBRARIES} ${EXTRA_LIBS} ${DEP_LIBS})
target_link_libraries(tocin-compiler ${LLVM_LIBRARIES} ${EXTRA_LIBS} ${DEP_LIBS})
target_link_libraries(tocin-compiler ${LLVM_LIBRARIES} ${EXTRA_LIBS} ${DEP_LIBS})





# Optionally, add tests and install targets here.
