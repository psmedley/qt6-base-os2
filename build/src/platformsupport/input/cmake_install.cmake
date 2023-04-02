# Install script for directory: /home/psmedley/qt6-base-os2/src/platformsupport/input

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local/Qt-6.4.2")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/input/meta_types/qt6inputsupportprivate_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6InputSupport.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtInputSupport/6.4.2/QtInputSupport/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/evdevkeyboard/qevdevkeyboard_defaultmap_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/evdevkeyboard/qevdevkeyboardhandler_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/evdevkeyboard/qevdevkeyboardmanager_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/evdevmouse/qevdevmousehandler_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/evdevmouse/qevdevmousemanager_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/evdevtablet/qevdevtablethandler_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/evdevtablet/qevdevtabletmanager_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/evdevtouch/qevdevtouchfilter_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/evdevtouch/qevdevtouchhandler_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/evdevtouch/qevdevtouchmanager_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/libinput/qlibinputhandler_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/libinput/qlibinputkeyboard_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/libinput/qlibinputpointer_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/libinput/qlibinputtouch_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/shared/devicehandlerlist_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/shared/qevdevutil_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/shared/qoutputmapping_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/tslib/qtslib_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtInputSupport" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/platformsupport/input/integrityhid/qintegrityhidmanager.h"
    "/home/psmedley/qt6-base-os2/build/include/QtInputSupport/QIntegrityHIDManager"
    "/home/psmedley/qt6-base-os2/build/include/QtInputSupport/qtinputsupportversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtInputSupport/QtInputSupportVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtInputSupport/QtInputSupport"
    "/home/psmedley/qt6-base-os2/build/include/QtInputSupport/QtInputSupportDepends"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/platformsupport/input/CMakeFiles/Export/9326bebf5fceb9819f7fd4baccc49af2/Qt6InputSupportPrivateTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/input/CMakeFiles/Export/9326bebf5fceb9819f7fd4baccc49af2/Qt6InputSupportPrivateTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/input/CMakeFiles/Export/9326bebf5fceb9819f7fd4baccc49af2/Qt6InputSupportPrivateTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/platformsupport/input/CMakeFiles/Export/9326bebf5fceb9819f7fd4baccc49af2/Qt6InputSupportPrivateVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/input/CMakeFiles/Export/9326bebf5fceb9819f7fd4baccc49af2/Qt6InputSupportPrivateVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/InputSupportPrivate.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6InputSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6InputSupportPrivate/Qt6InputSupportPrivateAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_input_support_private.pri")
endif()

