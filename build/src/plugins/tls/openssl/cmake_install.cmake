# Install script for directory: /home/psmedley/qt6-base-os2/src/plugins/tls/openssl

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

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Network" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Network/Qt6QTlsBackendOpenSSLPluginConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Network/Qt6QTlsBackendOpenSSLPluginConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Network/Qt6QTlsBackendOpenSSLPluginConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/tls/libqopensslbackend.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/tls/libqopensslbackend.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/tls/libqopensslbackend.so"
         RPATH "\$ORIGIN/../../lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./plugins/tls" TYPE MODULE FILES "/home/psmedley/qt6-base-os2/build/plugins/tls/libqopensslbackend.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/tls/libqopensslbackend.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/tls/libqopensslbackend.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/tls/libqopensslbackend.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN/../../lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./plugins/tls/libqopensslbackend.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Network/Qt6QTlsBackendOpenSSLPluginTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Network/Qt6QTlsBackendOpenSSLPluginTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/plugins/tls/openssl/CMakeFiles/Export/4f7a78213d2af5c8b0a2bc72fc885662/Qt6QTlsBackendOpenSSLPluginTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Network/Qt6QTlsBackendOpenSSLPluginTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Network/Qt6QTlsBackendOpenSSLPluginTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Network" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/tls/openssl/CMakeFiles/Export/4f7a78213d2af5c8b0a2bc72fc885662/Qt6QTlsBackendOpenSSLPluginTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Network" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/plugins/tls/openssl/CMakeFiles/Export/4f7a78213d2af5c8b0a2bc72fc885662/Qt6QTlsBackendOpenSSLPluginTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Network" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Network/Qt6QTlsBackendOpenSSLPluginAdditionalTargetInfo.cmake")
endif()
