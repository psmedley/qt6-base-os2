# Install script for directory: /home/psmedley/qt6-base-os2/src/platformsupport/kmsconvenience

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/kmsconvenience/meta_types/qt6kmssupportprivate_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6KmsSupport.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtKmsSupport/6.4.2/QtKmsSupport/private" TYPE FILE FILES "/home/psmedley/qt6-base-os2/src/platformsupport/kmsconvenience/qkmsdevice_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtKmsSupport" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtKmsSupport/qtkmssupportversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtKmsSupport/QtKmsSupportVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtKmsSupport/QtKmsSupport"
    "/home/psmedley/qt6-base-os2/build/include/QtKmsSupport/QtKmsSupportDepends"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/platformsupport/kmsconvenience/CMakeFiles/Export/b4724296e497172649befa49d7e133fa/Qt6KmsSupportPrivateTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/kmsconvenience/CMakeFiles/Export/b4724296e497172649befa49d7e133fa/Qt6KmsSupportPrivateTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/kmsconvenience/CMakeFiles/Export/b4724296e497172649befa49d7e133fa/Qt6KmsSupportPrivateTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/platformsupport/kmsconvenience/CMakeFiles/Export/b4724296e497172649befa49d7e133fa/Qt6KmsSupportPrivateVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/platformsupport/kmsconvenience/CMakeFiles/Export/b4724296e497172649befa49d7e133fa/Qt6KmsSupportPrivateVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/KmsSupportPrivate.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6KmsSupportPrivate" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6KmsSupportPrivate/Qt6KmsSupportPrivateAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_kms_support_private.pri")
endif()

