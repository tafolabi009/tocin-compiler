# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 4.0

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = C:\msys64\mingw64\bin\cmake.exe

# The command to remove a file.
RM = C:\msys64\mingw64\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\NT\Desktop\tocin-compiler\interpreter

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\NT\Desktop\tocin-compiler\interpreter\build

# Include any dependencies generated for this target.
include CMakeFiles/tocin-repl.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/tocin-repl.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/tocin-repl.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tocin-repl.dir/flags.make

CMakeFiles/tocin-repl.dir/codegen:
.PHONY : CMakeFiles/tocin-repl.dir/codegen

CMakeFiles/tocin-repl.dir/src/Repl.cpp.obj: CMakeFiles/tocin-repl.dir/flags.make
CMakeFiles/tocin-repl.dir/src/Repl.cpp.obj: CMakeFiles/tocin-repl.dir/includes_CXX.rsp
CMakeFiles/tocin-repl.dir/src/Repl.cpp.obj: C:/Users/NT/Desktop/tocin-compiler/interpreter/src/Repl.cpp
CMakeFiles/tocin-repl.dir/src/Repl.cpp.obj: CMakeFiles/tocin-repl.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\NT\Desktop\tocin-compiler\interpreter\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/tocin-repl.dir/src/Repl.cpp.obj"
	C:\msys64\mingw64\bin\c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tocin-repl.dir/src/Repl.cpp.obj -MF CMakeFiles\tocin-repl.dir\src\Repl.cpp.obj.d -o CMakeFiles\tocin-repl.dir\src\Repl.cpp.obj -c C:\Users\NT\Desktop\tocin-compiler\interpreter\src\Repl.cpp

CMakeFiles/tocin-repl.dir/src/Repl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/tocin-repl.dir/src/Repl.cpp.i"
	C:\msys64\mingw64\bin\c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\NT\Desktop\tocin-compiler\interpreter\src\Repl.cpp > CMakeFiles\tocin-repl.dir\src\Repl.cpp.i

CMakeFiles/tocin-repl.dir/src/Repl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/tocin-repl.dir/src/Repl.cpp.s"
	C:\msys64\mingw64\bin\c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\NT\Desktop\tocin-compiler\interpreter\src\Repl.cpp -o CMakeFiles\tocin-repl.dir\src\Repl.cpp.s

# Object files for target tocin-repl
tocin__repl_OBJECTS = \
"CMakeFiles/tocin-repl.dir/src/Repl.cpp.obj"

# External object files for target tocin-repl
tocin__repl_EXTERNAL_OBJECTS =

tocin-repl.exe: CMakeFiles/tocin-repl.dir/src/Repl.cpp.obj
tocin-repl.exe: CMakeFiles/tocin-repl.dir/build.make
tocin-repl.exe: libtocin_lib.a
tocin-repl.exe: C:/msys64/mingw64/lib/libboost_filesystem-mt.dll.a
tocin-repl.exe: CMakeFiles/tocin-repl.dir/linkLibs.rsp
tocin-repl.exe: CMakeFiles/tocin-repl.dir/objects1.rsp
tocin-repl.exe: CMakeFiles/tocin-repl.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Users\NT\Desktop\tocin-compiler\interpreter\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable tocin-repl.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\tocin-repl.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tocin-repl.dir/build: tocin-repl.exe
.PHONY : CMakeFiles/tocin-repl.dir/build

CMakeFiles/tocin-repl.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\tocin-repl.dir\cmake_clean.cmake
.PHONY : CMakeFiles/tocin-repl.dir/clean

CMakeFiles/tocin-repl.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\NT\Desktop\tocin-compiler\interpreter C:\Users\NT\Desktop\tocin-compiler\interpreter C:\Users\NT\Desktop\tocin-compiler\interpreter\build C:\Users\NT\Desktop\tocin-compiler\interpreter\build C:\Users\NT\Desktop\tocin-compiler\interpreter\build\CMakeFiles\tocin-repl.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/tocin-repl.dir/depend

