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

# Utility rule file for DeviceDiscoverySupportPrivate_version_script.

# Include any custom commands dependencies for this target.
include src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/compiler_depend.make

# Include the progress variables for this target.
include src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/progress.make

src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script: src/platformsupport/devicediscovery/DeviceDiscoverySupportPrivate.version

src/platformsupport/devicediscovery/DeviceDiscoverySupportPrivate.version: src/platformsupport/devicediscovery/DeviceDiscoverySupportPrivate.version.in
src/platformsupport/devicediscovery/DeviceDiscoverySupportPrivate.version: /home/psmedley/qt6-base-os2/mkspecs/features/data/unix/findclasslist.pl
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/psmedley/qt6-base-os2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating version linker script for target DeviceDiscoverySupportPrivate"
	cd /home/psmedley/qt6-base-os2/src/platformsupport/devicediscovery && /usr/bin/perl /home/psmedley/qt6-base-os2/mkspecs/features/data/unix/findclasslist.pl < /home/psmedley/qt6-base-os2/build/src/platformsupport/devicediscovery/DeviceDiscoverySupportPrivate.version.in > /home/psmedley/qt6-base-os2/build/src/platformsupport/devicediscovery/DeviceDiscoverySupportPrivate.version

DeviceDiscoverySupportPrivate_version_script: src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script
DeviceDiscoverySupportPrivate_version_script: src/platformsupport/devicediscovery/DeviceDiscoverySupportPrivate.version
DeviceDiscoverySupportPrivate_version_script: src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/build.make
.PHONY : DeviceDiscoverySupportPrivate_version_script

# Rule to build all files generated by this target.
src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/build: DeviceDiscoverySupportPrivate_version_script
.PHONY : src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/build

src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/clean:
	cd /home/psmedley/qt6-base-os2/build/src/platformsupport/devicediscovery && $(CMAKE_COMMAND) -P CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/cmake_clean.cmake
.PHONY : src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/clean

src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/depend:
	cd /home/psmedley/qt6-base-os2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/psmedley/qt6-base-os2 /home/psmedley/qt6-base-os2/src/platformsupport/devicediscovery /home/psmedley/qt6-base-os2/build /home/psmedley/qt6-base-os2/build/src/platformsupport/devicediscovery /home/psmedley/qt6-base-os2/build/src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/platformsupport/devicediscovery/CMakeFiles/DeviceDiscoverySupportPrivate_version_script.dir/depend
