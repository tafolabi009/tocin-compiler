cmake_minimum_required(VERSION 3.10)
project(tocin-compiler)

# -------------------------
# ZLIB Integration
# -------------------------
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
set(ZLIB_LIBRARY ${ZLIB_LIBRARIES})

# -------------------------
# LLVM Integration
# -------------------------
set(LLVM_DIR "C:/msys64/mingw64/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)
if(LLVM_FOUND)
    message(STATUS "Found LLVM version ${LLVM_VERSION}")
    message(STATUS "Using LLVMConfig.cmake from: ${LLVM_DIR}")
else()
    message(FATAL_ERROR "LLVM not found. Please ensure LLVM is installed and LLVM_DIR is set correctly.")
endif()
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

# -------------------------
# Python Integration
# -------------------------
set(Python_ROOT_DIR "C:/msys64/mingw64")
find_package(Python REQUIRED COMPONENTS Interpreter Development)
include_directories(${Python_INCLUDE_DIRS})

# -------------------------
# V8 Integration
# -------------------------
find_path(V8_INCLUDE_DIR v8.h HINTS "C:/msys64/mingw64/include")
find_library(V8_LIBRARY NAMES v8 v8.dll.a libv8 libv8.dll.a HINTS "C:/msys64/mingw64/lib")
if(V8_INCLUDE_DIR AND V8_LIBRARY AND NOT V8_LIBRARY STREQUAL "V8_LIBRARY-NOTFOUND")
    set(V8_FOUND TRUE)
    message(STATUS "Found V8 headers in: ${V8_INCLUDE_DIR}")
    message(STATUS "Found V8 library: ${V8_LIBRARY}")
    include_directories(${V8_INCLUDE_DIR})
else()
    set(V8_FOUND FALSE)
    message(WARNING "V8 not found. V8-related functionality will be disabled.")
endif()

# -------------------------
# FFI (libffi) Integration
# -------------------------
set(FFI_INCLUDE_DIR "C:/msys64/mingw64/include/libffi")
set(FFI_LIBRARY "C:/msys64/mingw64/lib/libffi.dll.a")
include_directories(${FFI_INCLUDE_DIR})

# -------------------------
# zstd Integration
# -------------------------
set(zstd_INCLUDE_DIR "C:/msys64/mingw64/include")
set(zstd_LIBRARY "C:/msys64/mingw64/lib/libzstd.dll.a")
include_directories(${zstd_INCLUDE_DIR})

# -------------------------
# LibXml2 Integration
# -------------------------
set(LibXml2_INCLUDE_DIR "C:/msys64/mingw64/include/libxml2")
set(LibXml2_LIBRARY "C:/msys64/mingw64/lib/libxml2.dll.a")
include_directories(${LibXml2_INCLUDE_DIR})

# -------------------------
# Source Files
# -------------------------
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(tocin-compiler ${SOURCES} "src/ffi/ffi_value.h")

# Set C++ standard to C++20 if supported
if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET tocin-compiler PROPERTY CXX_STANDARD 20)
endif()

# -------------------------
# Include Project Headers
# -------------------------
include_directories(${CMAKE_SOURCE_DIR}/src)

# -------------------------
# Linking Libraries
# -------------------------
llvm_map_components_to_libnames(llvm_libs
    core
    support
    passes
    target
    x86codegen
    x86asmparser
    asmprinter
    x86disassembler
    x86info
)
set(LINK_LIBS ${llvm_libs} ${Python_LIBRARIES} ${FFI_LIBRARY} ${ZLIB_LIBRARY} ${zstd_LIBRARY} ${LibXml2_LIBRARY})
if(V8_FOUND)
    list(APPEND LINK_LIBS ${V8_LIBRARY})
endif()
target_link_libraries(tocin-compiler PRIVATE ${LINK_LIBS})

# -------------------------
# CPack Installer Setup
# -------------------------
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_GENERATOR "NSIS")
set(CPACK_PACKAGE_NAME "tocin-compiler")
set(CPACK_PACKAGE_VENDOR "Afolabi_Oluwatosin")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tocin Compiler")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ToCin")

# Add compiler to system PATH
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_DISPLAY_NAME "Tocin Compiler")
set(CPACK_NSIS_PACKAGE_NAME "Tocin Compiler")
set(CPACK_NSIS_HELP_LINK "https://tocin-support.vercel.app/support")
set(CPACK_NSIS_URL_INFO_ABOUT "https://tocin-support.vercel.app")
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/Tocin_Logo.ico")

# Fixed NSIS extra install commands with proper escaping
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
    "SetOutPath \\\"$INSTDIR\\\\bin\\\"
     CreateDirectory \\\"$SMPROGRAMS\\\\tocin-compiler\\\"
     CreateShortCut \\\"$SMPROGRAMS\\\\tocin-compiler\\\\tocin-compiler.lnk\\\" \\\"$INSTDIR\\\\bin\\\\tocin-compiler.exe\\\"
     WriteRegStr HKLM \\\"SYSTEM\\\\CurrentControlSet\\\\Control\\\\Session Manager\\\\Environment\\\" \\\"PATH\\\" \\\"$INSTDIR\\\\bin;%PATH%\\\""
)

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
    "DeleteRegValue HKLM \\\"SYSTEM\\\\CurrentControlSet\\\\Control\\\\Session Manager\\\\Environment\\\" \\\"PATH\\\""
)

# -------------------------
# Install required DLLs
# -------------------------
install(FILES
    "C:/msys64/mingw64/bin/libwinpthread-1.dll"
    "C:/msys64/mingw64/bin/libstdc++-6.dll"
    "C:/msys64/mingw64/bin/libgcc_s_seh-1.dll"
    "C:/msys64/mingw64/bin/libpython3.12.dll"
    "C:/msys64/mingw64/bin/libv8.dll"
    "C:/msys64/mingw64/bin/zlib1.dll"
    "C:/msys64/mingw64/bin/libzstd.dll"
    "C:/msys64/mingw64/bin/libicudt76.dll"
    "C:/msys64/mingw64/bin/libicuin76.dll"
    "C:/msys64/mingw64/bin/libicuuc76.dll"
    "C:/msys64/mingw64/bin/libv8_libbase.dll"
    DESTINATION bin
)

install(TARGETS tocin-compiler DESTINATION bin)

# -------------------------
# CPack Final Setup
# -------------------------
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Tocin Compiler Application")
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Runtime Libraries")
set(CPACK_COMPONENT_APPLICATIONS_DEPENDS libraries)

include(CPack)
