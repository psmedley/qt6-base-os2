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
include src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/compiler_depend.make

# Include the progress variables for this target.
include src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/progress.make

# Include the compile flags for this target's objects.
include src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/flags.make

src/openglwidgets/meta_types/qt6openglwidgets_release_metatypes.json.gen: libexec/moc
src/openglwidgets/meta_types/qt6openglwidgets_release_metatypes.json.gen: src/openglwidgets/meta_types/OpenGLWidgets_json_file_list.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Running moc --collect-json for target OpenGLWidgets"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && ../../libexec/moc -o /home/psmedley/qt6-base-os2/build/src/openglwidgets/meta_types/qt6openglwidgets_release_metatypes.json.gen --collect-json @/home/psmedley/qt6-base-os2/build/src/openglwidgets/meta_types/OpenGLWidgets_json_file_list.txt
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/cmake -E copy_if_different /home/psmedley/qt6-base-os2/build/src/openglwidgets/meta_types/qt6openglwidgets_release_metatypes.json.gen /home/psmedley/qt6-base-os2/build/src/openglwidgets/meta_types/qt6openglwidgets_release_metatypes.json

src/openglwidgets/meta_types/qt6openglwidgets_release_metatypes.json: src/openglwidgets/meta_types/qt6openglwidgets_release_metatypes.json.gen
	@$(CMAKE_COMMAND) -E touch_nocreate src/openglwidgets/meta_types/qt6openglwidgets_release_metatypes.json

src/openglwidgets/preliminary_prl_for_OpenGLWidgets_step2_Release.prl: src/openglwidgets/preliminary_prl_for_OpenGLWidgets_step1_Release.prl
src/openglwidgets/preliminary_prl_for_OpenGLWidgets_step2_Release.prl: src/openglwidgets/preliminary_prl_meta_info_for_OpenGLWidgets_Release.txt
src/openglwidgets/preliminary_prl_for_OpenGLWidgets_step2_Release.prl: /home/psmedley/qt6-base-os2/cmake/QtFinishPrlFile.cmake
src/openglwidgets/preliminary_prl_for_OpenGLWidgets_step2_Release.prl: /home/psmedley/qt6-base-os2/cmake/QtGenerateLibHelpers.cmake
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating prl file for target OpenGLWidgets"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/cmake -DIN_FILE=/home/psmedley/qt6-base-os2/build/src/openglwidgets/preliminary_prl_for_OpenGLWidgets_step1_Release.prl -DIN_META_FILE=/home/psmedley/qt6-base-os2/build/src/openglwidgets/preliminary_prl_meta_info_for_OpenGLWidgets_Release.txt -DOUT_FILE=/home/psmedley/qt6-base-os2/build/src/openglwidgets/preliminary_prl_for_OpenGLWidgets_step2_Release.prl "-DLIBRARY_PREFIXES=lib;lib" "-DLIBRARY_SUFFIXES=.so;.a" -DLINK_LIBRARY_FLAG=-l "-DQT_LIB_DIRS=/home/psmedley/qt6-base-os2/build/lib;/usr/local/Qt-6.4.2/lib" "-DQT_PLUGIN_DIRS=/home/psmedley/qt6-base-os2/build/./plugins;/usr/local/Qt-6.4.2/./plugins" "-DQT_QML_DIRS=/home/psmedley/qt6-base-os2/build/./qml;/usr/local/Qt-6.4.2/./qml" "-DIMPLICIT_LINK_DIRECTORIES=/usr/lib/gcc/x86_64-linux-gnu/12;/usr/lib/x86_64-linux-gnu;/usr/lib;/lib/x86_64-linux-gnu;/lib" -P /home/psmedley/qt6-base-os2/cmake/QtFinishPrlFile.cmake

lib/pkgconfig/Qt6OpenGLWidgets.pc: lib/pkgconfig/preliminary_pc_for_Qt6OpenGLWidgets_Release_step2.pc
lib/pkgconfig/Qt6OpenGLWidgets.pc: /home/psmedley/qt6-base-os2/cmake/QtFinishPkgConfigFile.cmake
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Generating pc file for target Qt6::OpenGLWidgets"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/cmake -DIN_FILE=/home/psmedley/qt6-base-os2/build/lib/pkgconfig/preliminary_pc_for_Qt6OpenGLWidgets_Release_step2.pc -DOUT_FILE=/home/psmedley/qt6-base-os2/build/lib/pkgconfig/Qt6OpenGLWidgets.pc -P /home/psmedley/qt6-base-os2/cmake/QtFinishPkgConfigFile.cmake

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/flags.make
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o: src/openglwidgets/OpenGLWidgets_autogen/mocs_compilation.cpp
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -include /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx -MD -MT src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o -MF CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o.d -o CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o -c /home/psmedley/qt6-base-os2/build/src/openglwidgets/OpenGLWidgets_autogen/mocs_compilation.cpp

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.i"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -include /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx -E /home/psmedley/qt6-base-os2/build/src/openglwidgets/OpenGLWidgets_autogen/mocs_compilation.cpp > CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.i

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.s"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -include /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx -S /home/psmedley/qt6-base-os2/build/src/openglwidgets/OpenGLWidgets_autogen/mocs_compilation.cpp -o CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.s

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/flags.make
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.cxx
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -x c++-header -include /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx -MD -MT src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch -MF CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch.d -o CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch -c /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.cxx

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.i"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -x c++-header -include /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx -E /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.cxx > CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.i

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.s"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -x c++-header -include /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx -S /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.cxx -o CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.s

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/flags.make
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o: /home/psmedley/qt6-base-os2/src/openglwidgets/qopenglwidget.cpp
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -include /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx -MD -MT src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o -MF CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o.d -o CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o -c /home/psmedley/qt6-base-os2/src/openglwidgets/qopenglwidget.cpp

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.i"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -include /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx -E /home/psmedley/qt6-base-os2/src/openglwidgets/qopenglwidget.cpp > CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.i

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.s"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -Winvalid-pch -include /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx -S /home/psmedley/qt6-base-os2/src/openglwidgets/qopenglwidget.cpp -o CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.s

# Object files for target OpenGLWidgets
OpenGLWidgets_OBJECTS = \
"CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o" \
"CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o"

# External object files for target OpenGLWidgets
OpenGLWidgets_EXTERNAL_OBJECTS =

lib/libQt6OpenGLWidgets.so.6.4.2: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/OpenGLWidgets_autogen/mocs_compilation.cpp.o
lib/libQt6OpenGLWidgets.so.6.4.2: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/cmake_pch.hxx.gch
lib/libQt6OpenGLWidgets.so.6.4.2: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/qopenglwidget.cpp.o
lib/libQt6OpenGLWidgets.so.6.4.2: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/build.make
lib/libQt6OpenGLWidgets.so.6.4.2: /usr/lib/x86_64-linux-gnu/libGLX.so
lib/libQt6OpenGLWidgets.so.6.4.2: /usr/lib/x86_64-linux-gnu/libOpenGL.so
lib/libQt6OpenGLWidgets.so.6.4.2: /usr/lib/x86_64-linux-gnu/libxkbcommon.so
lib/libQt6OpenGLWidgets.so.6.4.2: src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX shared library ../../lib/libQt6OpenGLWidgets.so"
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/OpenGLWidgets.dir/link.txt --verbose=$(VERBOSE)
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && $(CMAKE_COMMAND) -E cmake_symlink_library ../../lib/libQt6OpenGLWidgets.so.6.4.2 ../../lib/libQt6OpenGLWidgets.so.6 ../../lib/libQt6OpenGLWidgets.so

lib/libQt6OpenGLWidgets.so.6: lib/libQt6OpenGLWidgets.so.6.4.2
	@$(CMAKE_COMMAND) -E touch_nocreate lib/libQt6OpenGLWidgets.so.6

lib/libQt6OpenGLWidgets.so: lib/libQt6OpenGLWidgets.so.6.4.2
	@$(CMAKE_COMMAND) -E touch_nocreate lib/libQt6OpenGLWidgets.so

# Rule to build all files generated by this target.
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/build: lib/libQt6OpenGLWidgets.so
.PHONY : src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/build

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/clean:
	cd /home/psmedley/qt6-base-os2/build/src/openglwidgets && $(CMAKE_COMMAND) -P CMakeFiles/OpenGLWidgets.dir/cmake_clean.cmake
.PHONY : src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/clean

src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/depend: lib/pkgconfig/Qt6OpenGLWidgets.pc
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/depend: src/openglwidgets/meta_types/qt6openglwidgets_release_metatypes.json
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/depend: src/openglwidgets/meta_types/qt6openglwidgets_release_metatypes.json.gen
src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/depend: src/openglwidgets/preliminary_prl_for_OpenGLWidgets_step2_Release.prl
	cd /home/psmedley/qt6-base-os2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/psmedley/qt6-base-os2 /home/psmedley/qt6-base-os2/src/openglwidgets /home/psmedley/qt6-base-os2/build /home/psmedley/qt6-base-os2/build/src/openglwidgets /home/psmedley/qt6-base-os2/build/src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/openglwidgets/CMakeFiles/OpenGLWidgets.dir/depend
