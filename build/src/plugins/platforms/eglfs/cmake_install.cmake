# Install script for directory: /home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/meta_types/qt6eglfsdeviceintegrationprivate_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/psmedley/qt6-base-os2/build/lib/libQt6EglFSDeviceIntegration.so.6.4.2"
    "/home/psmedley/qt6-base-os2/build/lib/libQt6EglFSDeviceIntegration.so.6"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
           NEW_RPATH "\$ORIGIN")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so"
         RPATH "\$ORIGIN")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6EglFSDeviceIntegration.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFSDeviceIntegration.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtEglFSDeviceIntegration/6.4.2/QtEglFSDeviceIntegration/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/api/qeglfscontext_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/api/qeglfscursor_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/api/qeglfsdeviceintegration_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/api/qeglfsglobal_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/api/qeglfshooks_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/api/qeglfsintegration_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/api/qeglfsoffscreenwindow_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/api/qeglfsscreen_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/api/qeglfswindow_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtEglFSDeviceIntegration" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtEglFSDeviceIntegration/qteglfsdeviceintegrationversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtEglFSDeviceIntegration/QtEglFSDeviceIntegrationVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtEglFSDeviceIntegration/QtEglFSDeviceIntegration"
    "/home/psmedley/qt6-base-os2/build/include/QtEglFSDeviceIntegration/QtEglFSDeviceIntegrationDepends"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/CMakeFiles/Export/a6bab45806326d4b121448c61b117ae2/Qt6EglFSDeviceIntegrationPrivateTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/CMakeFiles/Export/a6bab45806326d4b121448c61b117ae2/Qt6EglFSDeviceIntegrationPrivateTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/CMakeFiles/Export/a6bab45806326d4b121448c61b117ae2/Qt6EglFSDeviceIntegrationPrivateTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/CMakeFiles/Export/a6bab45806326d4b121448c61b117ae2/Qt6EglFSDeviceIntegrationPrivateVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate/Qt6EglFSDeviceIntegrationPrivateVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFSDeviceIntegrationPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/CMakeFiles/Export/a6bab45806326d4b121448c61b117ae2/Qt6EglFSDeviceIntegrationPrivateVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/EglFSDeviceIntegrationPrivate.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6QEglFSIntegrationPluginConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6QEglFSIntegrationPluginConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6QEglFSIntegrationPluginConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/platforms/libqeglfs.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/platforms/libqeglfs.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/platforms/libqeglfs.so"
         RPATH "\$ORIGIN/../../lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./plugins/platforms" TYPE MODULE FILES "/home/psmedley/qt6-base-os2/build/plugins/platforms/libqeglfs.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/platforms/libqeglfs.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/platforms/libqeglfs.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/platforms/libqeglfs.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN/../../lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/platforms/libqeglfs.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6QEglFSIntegrationPluginTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6QEglFSIntegrationPluginTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6QEglFSIntegrationPluginTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6QEglFSIntegrationPluginTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6QEglFSIntegrationPluginTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6QEglFSIntegrationPluginTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6QEglFSIntegrationPluginTargets-release.cmake")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/deviceintegration/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_eglfsdeviceintegration_private.pri")
endif()

