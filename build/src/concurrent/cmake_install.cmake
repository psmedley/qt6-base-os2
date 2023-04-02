# Install script for directory: /home/psmedley/qt6-base-os2/src/concurrent

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtConcurrent" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/include/QtConcurrent/qtconcurrentexports.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/concurrent/meta_types/qt6concurrent_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Concurrent/Qt6ConcurrentConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Concurrent/Qt6ConcurrentConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Concurrent/Qt6ConcurrentConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/psmedley/qt6-base-os2/build/lib/libQt6Concurrent.so.6.4.2"
    "/home/psmedley/qt6-base-os2/build/lib/libQt6Concurrent.so.6"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so.6"
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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so"
         RPATH "\$ORIGIN")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6Concurrent.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Concurrent.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtConcurrent" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtConcurrent/qtconcurrentexports.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtaskbuilder.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrent_global.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentcompilertest.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentfilter.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentfilterkernel.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentfunctionwrappers.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentiteratekernel.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentmap.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentmapkernel.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentmedian.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentreducekernel.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentrun.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentrunbase.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentstoredfunctioncall.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrenttask.h"
    "/home/psmedley/qt6-base-os2/src/concurrent/qtconcurrentthreadengine.h"
    "/home/psmedley/qt6-base-os2/build/include/QtConcurrent/QtConcurrentFilter"
    "/home/psmedley/qt6-base-os2/build/include/QtConcurrent/QtConcurrentMap"
    "/home/psmedley/qt6-base-os2/build/include/QtConcurrent/QtConcurrentRun"
    "/home/psmedley/qt6-base-os2/build/include/QtConcurrent/qtconcurrentversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtConcurrent/QtConcurrentVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtConcurrent/QtConcurrent"
    "/home/psmedley/qt6-base-os2/build/include/QtConcurrent/QtConcurrentDepends"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent/Qt6ConcurrentTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent/Qt6ConcurrentTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/concurrent/CMakeFiles/Export/bade3f6fd9bf9bb0f7601ee42ed4bfdb/Qt6ConcurrentTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent/Qt6ConcurrentTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent/Qt6ConcurrentTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/concurrent/CMakeFiles/Export/bade3f6fd9bf9bb0f7601ee42ed4bfdb/Qt6ConcurrentTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/concurrent/CMakeFiles/Export/bade3f6fd9bf9bb0f7601ee42ed4bfdb/Qt6ConcurrentTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent/Qt6ConcurrentVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent/Qt6ConcurrentVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/concurrent/CMakeFiles/Export/bade3f6fd9bf9bb0f7601ee42ed4bfdb/Qt6ConcurrentVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent/Qt6ConcurrentVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent/Qt6ConcurrentVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/concurrent/CMakeFiles/Export/bade3f6fd9bf9bb0f7601ee42ed4bfdb/Qt6ConcurrentVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/Concurrent.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_html_docs_Concurrent")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc/qtconcurrent" TYPE DIRECTORY FILES "/home/psmedley/qt6-base-os2/build/./doc/qtconcurrent/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_qch_docs_Concurrent")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./doc/qtconcurrent.qch")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Concurrent" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Concurrent/Qt6ConcurrentAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_concurrent.pri"
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_concurrent_private.pri"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/pkgconfig/Qt6Concurrent.pc")
endif()

