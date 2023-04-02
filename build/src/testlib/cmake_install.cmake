# Install script for directory: /home/psmedley/qt6-base-os2/src/testlib

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtTest" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/include/QtTest/qttestexports.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtTest" TYPE FILE OPTIONAL RENAME "qttestlib-config.h" FILES "/home/psmedley/qt6-base-os2/build/src/testlib/qttestlib-config.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtTest/6.4.2/QtTest/private" TYPE FILE OPTIONAL RENAME "qttestlib-config_p.h" FILES "/home/psmedley/qt6-base-os2/build/src/testlib/qttestlib-config_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/testlib/meta_types/qt6test_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Test/Qt6TestConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Test/Qt6TestConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Test/Qt6TestConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/psmedley/qt6-base-os2/build/lib/libQt6Test.so.6.4.2"
    "/home/psmedley/qt6-base-os2/build/lib/libQt6Test.so.6"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so.6"
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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so"
         RPATH "\$ORIGIN")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6Test.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Test.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtTest/6.4.2/QtTest/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/testlib/qabstracttestlogger_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qappletestlogger_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qbenchmark_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qbenchmarkevent_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qbenchmarkmeasurement_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qbenchmarkmetric_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qbenchmarkperfevents_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qbenchmarktimemeasurers_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qbenchmarkvalgrind_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qcsvbenchmarklogger_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qemulationdetector_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qjunittestlogger_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qplaintestlogger_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qpropertytesthelper_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qsignaldumper_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtaptestlogger_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qteamcitylogger_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestblacklist_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestcoreelement_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestelement_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestelementattribute_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtesthelpers_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestjunitstreamer_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestlog_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestresult_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtesttable_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestutil_macos_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qxctestlogger_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qxmltestlogger_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/3rdparty/callgrind_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/3rdparty/cycle_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/3rdparty/linux_perf_event_p.h"
    "/home/psmedley/qt6-base-os2/src/testlib/3rdparty/valgrind_p.h"
    "/home/psmedley/qt6-base-os2/build/src/testlib/qttestlib-config_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtTest" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtTest/qttestexports.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qabstractitemmodeltester.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qbenchmark.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qbenchmarkmetric.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qsignalspy.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtest.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtest_gui.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtest_network.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtest_widgets.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestaccessible.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestassert.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestcase.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestdata.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestevent.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtesteventloop.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestkeyboard.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestmouse.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestspontaneevent.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtestsystem.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qtesttouch.h"
    "/home/psmedley/qt6-base-os2/src/testlib/qttestglobal.h"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QAbstractItemModelTester"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QSignalSpy"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QtCoverageScanner"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QTest"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QtTestGui"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QtTestNetwork"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QtTestWidgets"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QTestAccessibility"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QTestData"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QTestEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QTestKeyEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QTestKeyClicksEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QTestMouseEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QTestDelayEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QTestEventList"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QTestEventLoop"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QSpontaneKeyEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/qtest_global.h"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/qttestversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QtTestVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QtTest"
    "/home/psmedley/qt6-base-os2/build/include/QtTest/QtTestDepends"
    "/home/psmedley/qt6-base-os2/build/src/testlib/qttestlib-config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test/Qt6TestTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test/Qt6TestTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/testlib/CMakeFiles/Export/e0e8a5215ca67046dcc96cd802132d6a/Qt6TestTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test/Qt6TestTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test/Qt6TestTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/testlib/CMakeFiles/Export/e0e8a5215ca67046dcc96cd802132d6a/Qt6TestTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/testlib/CMakeFiles/Export/e0e8a5215ca67046dcc96cd802132d6a/Qt6TestTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test/Qt6TestVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test/Qt6TestVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/testlib/CMakeFiles/Export/e0e8a5215ca67046dcc96cd802132d6a/Qt6TestVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test/Qt6TestVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test/Qt6TestVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/testlib/CMakeFiles/Export/e0e8a5215ca67046dcc96cd802132d6a/Qt6TestVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/Test.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_html_docs_Test")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc/qttestlib" TYPE DIRECTORY FILES "/home/psmedley/qt6-base-os2/build/./doc/qttestlib/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_qch_docs_Test")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./doc/qttestlib.qch")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Test" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Test/Qt6TestAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_testlib.pri"
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_testlib_private.pri"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/pkgconfig/Qt6Test.pc")
endif()

