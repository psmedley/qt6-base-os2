# Install script for directory: /home/psmedley/qt6-base-os2/src/platformsupport/fbconvenience

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/fbconvenience/meta_types/qt6fbsupportprivate_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6FbSupport.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtFbSupport/6.4.2/QtFbSupport/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/platformsupport/fbconvenience/qfbbackingstore_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/fbconvenience/qfbcursor_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/fbconvenience/qfbscreen_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/fbconvenience/qfbvthandler_p.h"
    "/home/psmedley/qt6-base-os2/src/platformsupport/fbconvenience/qfbwindow_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtFbSupport" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtFbSupport/qtfbsupportversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtFbSupport/QtFbSupportVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtFbSupport/QtFbSupport"
    "/home/psmedley/qt6-base-os2/build/include/QtFbSupport/QtFbSupportDepends"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/platformsupport/fbconvenience/CMakeFiles/Export/6ebe586314b3c44bf5dfc43e96699207/Qt6FbSupportPrivateTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/fbconvenience/CMakeFiles/Export/6ebe586314b3c44bf5dfc43e96699207/Qt6FbSupportPrivateTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/fbconvenience/CMakeFiles/Export/6ebe586314b3c44bf5dfc43e96699207/Qt6FbSupportPrivateTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/platformsupport/fbconvenience/CMakeFiles/Export/6ebe586314b3c44bf5dfc43e96699207/Qt6FbSupportPrivateVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/fbconvenience/CMakeFiles/Export/6ebe586314b3c44bf5dfc43e96699207/Qt6FbSupportPrivateVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/FbSupportPrivate.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6FbSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6FbSupportPrivate/Qt6FbSupportPrivateAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_fb_support_private.pri")
endif()
