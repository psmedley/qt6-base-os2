# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/psmedley/qt6-base-os2/config.tests/x86_simd

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/psmedley/qt6-base-os2/build/config.tests/x86_simd_avx512dq

# Include any dependencies generated for this target.
include CMakeFiles/SimdTestavx512dq.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/SimdTestavx512dq.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/SimdTestavx512dq.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/SimdTestavx512dq.dir/flags.make

CMakeFiles/SimdTestavx512dq.dir/main.cpp.o: CMakeFiles/SimdTestavx512dq.dir/flags.make
CMakeFiles/SimdTestavx512dq.dir/main.cpp.o: /home/psmedley/qt6-base-os2/config.tests/x86_simd/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/psmedley/qt6-base-os2/build/config.tests/x86_simd_avx512dq/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/SimdTestavx512dq.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/SimdTestavx512dq.dir/main.cpp.o -c /home/psmedley/qt6-base-os2/config.tests/x86_simd/main.cpp

CMakeFiles/SimdTestavx512dq.dir/main.cpp.i: cmake_force
	@echo "Preprocessing CXX source to CMakeFiles/SimdTestavx512dq.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/psmedley/qt6-base-os2/config.tests/x86_simd/main.cpp > CMakeFiles/SimdTestavx512dq.dir/main.cpp.i

CMakeFiles/SimdTestavx512dq.dir/main.cpp.s: cmake_force
	@echo "Compiling CXX source to assembly CMakeFiles/SimdTestavx512dq.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/psmedley/qt6-base-os2/config.tests/x86_simd/main.cpp -o CMakeFiles/SimdTestavx512dq.dir/main.cpp.s

# Object files for target SimdTestavx512dq
SimdTestavx512dq_OBJECTS = \
"CMakeFiles/SimdTestavx512dq.dir/main.cpp.o"

# External object files for target SimdTestavx512dq
SimdTestavx512dq_EXTERNAL_OBJECTS =

SimdTestavx512dq: CMakeFiles/SimdTestavx512dq.dir/main.cpp.o
SimdTestavx512dq: CMakeFiles/SimdTestavx512dq.dir/build.make
SimdTestavx512dq: CMakeFiles/SimdTestavx512dq.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --progress-dir=/home/psmedley/qt6-base-os2/build/config.tests/x86_simd_avx512dq/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable SimdTestavx512dq"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/SimdTestavx512dq.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/SimdTestavx512dq.dir/build: SimdTestavx512dq
.PHONY : CMakeFiles/SimdTestavx512dq.dir/build

CMakeFiles/SimdTestavx512dq.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/SimdTestavx512dq.dir/cmake_clean.cmake
.PHONY : CMakeFiles/SimdTestavx512dq.dir/clean

CMakeFiles/SimdTestavx512dq.dir/depend:
	cd /home/psmedley/qt6-base-os2/build/config.tests/x86_simd_avx512dq && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/psmedley/qt6-base-os2/config.tests/x86_simd /home/psmedley/qt6-base-os2/config.tests/x86_simd /home/psmedley/qt6-base-os2/build/config.tests/x86_simd_avx512dq /home/psmedley/qt6-base-os2/build/config.tests/x86_simd_avx512dq /home/psmedley/qt6-base-os2/build/config.tests/x86_simd_avx512dq/CMakeFiles/SimdTestavx512dq.dir/DependInfo.cmake
.PHONY : CMakeFiles/SimdTestavx512dq.dir/depend
