# Install script for directory: /home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/meta_types/qt6eglfskmsgbmsupportprivate_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/psmedley/qt6-base-os2/build/lib/libQt6EglFsKmsGbmSupport.so.6.4.2"
    "/home/psmedley/qt6-base-os2/build/lib/libQt6EglFsKmsGbmSupport.so.6"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so.6"
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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so"
         RPATH "\$ORIGIN")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6EglFsKmsGbmSupport.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6EglFsKmsGbmSupport.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtEglFsKmsGbmSupport/6.4.2/QtEglFsKmsGbmSupport/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/qeglfskmsgbmcursor_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/qeglfskmsgbmdevice_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/qeglfskmsgbmintegration_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/qeglfskmsgbmscreen_p.h"
    "/home/psmedley/qt6-base-os2/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/qeglfskmsgbmwindow_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtEglFsKmsGbmSupport" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtEglFsKmsGbmSupport/qteglfskmsgbmsupportversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtEglFsKmsGbmSupport/QtEglFsKmsGbmSupportVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtEglFsKmsGbmSupport/QtEglFsKmsGbmSupport"
    "/home/psmedley/qt6-base-os2/build/include/QtEglFsKmsGbmSupport/QtEglFsKmsGbmSupportDepends"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/CMakeFiles/Export/0ae1fbb0a08b27401e40ca33d045ec91/Qt6EglFsKmsGbmSupportPrivateTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/CMakeFiles/Export/0ae1fbb0a08b27401e40ca33d045ec91/Qt6EglFsKmsGbmSupportPrivateTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/CMakeFiles/Export/0ae1fbb0a08b27401e40ca33d045ec91/Qt6EglFsKmsGbmSupportPrivateTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/CMakeFiles/Export/0ae1fbb0a08b27401e40ca33d045ec91/Qt6EglFsKmsGbmSupportPrivateVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/CMakeFiles/Export/0ae1fbb0a08b27401e40ca33d045ec91/Qt6EglFsKmsGbmSupportPrivateVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/EglFsKmsGbmSupportPrivate.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6QEglFSKmsGbmIntegrationPluginConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6QEglFSKmsGbmIntegrationPluginConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6QEglFSKmsGbmIntegrationPluginConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/egldeviceintegrations/libqeglfs-kms-integration.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/egldeviceintegrations/libqeglfs-kms-integration.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/egldeviceintegrations/libqeglfs-kms-integration.so"
         RPATH "\$ORIGIN/../../lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./plugins/egldeviceintegrations" TYPE MODULE FILES "/home/psmedley/qt6-base-os2/build/plugins/egldeviceintegrations/libqeglfs-kms-integration.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/egldeviceintegrations/libqeglfs-kms-integration.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/egldeviceintegrations/libqeglfs-kms-integration.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/egldeviceintegrations/libqeglfs-kms-integration.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN/../../lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/egldeviceintegrations/libqeglfs-kms-integration.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6QEglFSKmsGbmIntegrationPluginTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6QEglFSKmsGbmIntegrationPluginTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6QEglFSKmsGbmIntegrationPluginTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6QEglFSKmsGbmIntegrationPluginTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6QEglFSKmsGbmIntegrationPluginTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6QEglFSKmsGbmIntegrationPluginTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/platforms/eglfs/deviceintegration/eglfs_kms/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6QEglFSKmsGbmIntegrationPluginTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6EglFsKmsGbmSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6EglFsKmsGbmSupportPrivate/Qt6EglFsKmsGbmSupportPrivateAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6QEglFSKmsGbmIntegrationPluginAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_eglfs_kms_gbm_support_private.pri")
endif()

