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
CMAKE_SOURCE_DIR = /home/psmedley/qt6-base-os2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/psmedley/qt6-base-os2/build

# Include any dependencies generated for this target.
include src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/compiler_depend.make

# Include the progress variables for this target.
include src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/progress.make

# Include the compile flags for this target's objects.
include src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/flags.make

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.o: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/flags.make
src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.o: src/plugins/platforms/vkkhrdisplay/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp
src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.o: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.o"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.o -MF CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.o.d -o CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.o -c /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.i"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp > CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.i

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.s"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp -o CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.s

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.o: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/flags.make
src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.o: /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/main.cpp
src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.o: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.o"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.o -MF CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.o.d -o CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.o -c /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/main.cpp

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.i"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/main.cpp > CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.i

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.s"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/main.cpp -o CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.s

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.o: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/flags.make
src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.o: /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/qvkkhrdisplayintegration.cpp
src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.o: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.o"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.o -MF CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.o.d -o CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.o -c /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/qvkkhrdisplayintegration.cpp

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.i"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/qvkkhrdisplayintegration.cpp > CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.i

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.s"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/qvkkhrdisplayintegration.cpp -o CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.s

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.o: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/flags.make
src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.o: /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/qvkkhrdisplayvulkaninstance.cpp
src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.o: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.o"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.o -MF CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.o.d -o CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.o -c /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/qvkkhrdisplayvulkaninstance.cpp

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.i"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/qvkkhrdisplayvulkaninstance.cpp > CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.i

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.s"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay/qvkkhrdisplayvulkaninstance.cpp -o CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.s

# Object files for target QVkKhrDisplayIntegrationPlugin
QVkKhrDisplayIntegrationPlugin_OBJECTS = \
"CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.o" \
"CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.o" \
"CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.o" \
"CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.o"

# External object files for target QVkKhrDisplayIntegrationPlugin
QVkKhrDisplayIntegrationPlugin_EXTERNAL_OBJECTS =

plugins/platforms/libqvkkhrdisplay.so: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/QVkKhrDisplayIntegrationPlugin_autogen/mocs_compilation.cpp.o
plugins/platforms/libqvkkhrdisplay.so: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/main.cpp.o
plugins/platforms/libqvkkhrdisplay.so: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayintegration.cpp.o
plugins/platforms/libqvkkhrdisplay.so: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/qvkkhrdisplayvulkaninstance.cpp.o
plugins/platforms/libqvkkhrdisplay.so: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/build.make
plugins/platforms/libqvkkhrdisplay.so: lib/libQt6FbSupport.a
plugins/platforms/libqvkkhrdisplay.so: lib/libQt6InputSupport.a
plugins/platforms/libqvkkhrdisplay.so: /usr/lib/x86_64-linux-gnu/libfreetype.so
plugins/platforms/libqvkkhrdisplay.so: /usr/lib/x86_64-linux-gnu/libGLX.so
plugins/platforms/libqvkkhrdisplay.so: /usr/lib/x86_64-linux-gnu/libOpenGL.so
plugins/platforms/libqvkkhrdisplay.so: /usr/lib/x86_64-linux-gnu/libxkbcommon.so
plugins/platforms/libqvkkhrdisplay.so: lib/libQt6DeviceDiscoverySupport.a
plugins/platforms/libqvkkhrdisplay.so: /usr/lib/x86_64-linux-gnu/libudev.so
plugins/platforms/libqvkkhrdisplay.so: /usr/lib/x86_64-linux-gnu/libmtdev.so
plugins/platforms/libqvkkhrdisplay.so: /usr/lib/x86_64-linux-gnu/libinput.so
plugins/platforms/libqvkkhrdisplay.so: src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX shared module ../../../../plugins/platforms/libqvkkhrdisplay.so"
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/build: plugins/platforms/libqvkkhrdisplay.so
.PHONY : src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/build

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/clean:
	cd /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay && $(CMAKE_COMMAND) -P CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/cmake_clean.cmake
.PHONY : src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/clean

src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/depend:
	cd /home/psmedley/qt6-base-os2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/psmedley/qt6-base-os2 /home/psmedley/qt6-base-os2/src/plugins/platforms/vkkhrdisplay /home/psmedley/qt6-base-os2/build /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay /home/psmedley/qt6-base-os2/build/src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/plugins/platforms/vkkhrdisplay/CMakeFiles/QVkKhrDisplayIntegrationPlugin.dir/depend
