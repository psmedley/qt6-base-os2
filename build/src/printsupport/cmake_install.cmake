# Install script for directory: /home/psmedley/qt6-base-os2/src/printsupport

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtPrintSupport" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/qtprintsupportexports.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtPrintSupport/6.4.2/QtPrintSupport/qpa" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qplatformprintdevice.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qplatformprintersupport.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qplatformprintplugin.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtPrintSupport" TYPE FILE OPTIONAL RENAME "qtprintsupport-config.h" FILES "/home/psmedley/qt6-base-os2/build/src/printsupport/qtprintsupport-config.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtPrintSupport/6.4.2/QtPrintSupport/private" TYPE FILE OPTIONAL RENAME "qtprintsupport-config_p.h" FILES "/home/psmedley/qt6-base-os2/build/src/printsupport/qtprintsupport-config_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/printsupport/meta_types/qt6printsupport_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6PrintSupport/Qt6PrintSupportConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6PrintSupport/Qt6PrintSupportConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6PrintSupport/Qt6PrintSupportConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/psmedley/qt6-base-os2/build/lib/libQt6PrintSupport.so.6.4.2"
    "/home/psmedley/qt6-base-os2/build/lib/libQt6PrintSupport.so.6"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so.6"
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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so"
         RPATH "\$ORIGIN")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6PrintSupport.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6PrintSupport.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtPrintSupport/6.4.2/QtPrintSupport/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/printsupport/dialogs/qabstractprintdialog_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/dialogs/qpagesetupdialog_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/dialogs/qpagesetupdialog_unix_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qcups_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qpaintengine_alpha_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qpaintengine_preview_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qprint_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qprintdevice_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qprintengine_pdf_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qprinter_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qprinterinfo_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qtprintsupportglobal_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/widgets/qcupsjobwidget_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/platform/macos/qcocoaprintdevice_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/platform/macos/qcocoaprintersupport_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/platform/macos/qpaintengine_mac_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/platform/macos/qprintengine_mac_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/platform/windows/qprintengine_win_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/platform/windows/qwindowsprintdevice_p.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/platform/windows/qwindowsprintersupport_p.h"
    "/home/psmedley/qt6-base-os2/build/src/printsupport/qtprintsupport-config_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtPrintSupport" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/qtprintsupportexports.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/dialogs/qabstractprintdialog.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/dialogs/qpagesetupdialog.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/dialogs/qprintdialog.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/dialogs/qprintpreviewdialog.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qprintengine.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qprinter.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qprinterinfo.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/kernel/qtprintsupportglobal.h"
    "/home/psmedley/qt6-base-os2/src/printsupport/widgets/qprintpreviewwidget.h"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QAbstractPrintDialog"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QPageSetupDialog"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QPrintDialog"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QPrintPreviewDialog"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QPrintEngine"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QPrinter"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QPrinterInfo"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QPrintPreviewWidget"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/qtprintsupportversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QtPrintSupportVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QtPrintSupport"
    "/home/psmedley/qt6-base-os2/build/include/QtPrintSupport/QtPrintSupportDepends"
    "/home/psmedley/qt6-base-os2/build/src/printsupport/qtprintsupport-config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport/Qt6PrintSupportTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport/Qt6PrintSupportTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/printsupport/CMakeFiles/Export/893f683a1d7948d2e0ede002b596fc68/Qt6PrintSupportTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport/Qt6PrintSupportTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport/Qt6PrintSupportTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/printsupport/CMakeFiles/Export/893f683a1d7948d2e0ede002b596fc68/Qt6PrintSupportTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/printsupport/CMakeFiles/Export/893f683a1d7948d2e0ede002b596fc68/Qt6PrintSupportTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport/Qt6PrintSupportVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport/Qt6PrintSupportVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/printsupport/CMakeFiles/Export/893f683a1d7948d2e0ede002b596fc68/Qt6PrintSupportVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport/Qt6PrintSupportVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport/Qt6PrintSupportVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/printsupport/CMakeFiles/Export/893f683a1d7948d2e0ede002b596fc68/Qt6PrintSupportVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/PrintSupport.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_html_docs_PrintSupport")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc/qtprintsupport" TYPE DIRECTORY FILES "/home/psmedley/qt6-base-os2/build/./doc/qtprintsupport/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_qch_docs_PrintSupport")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./doc/qtprintsupport.qch")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6PrintSupport" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6PrintSupport/Qt6PrintSupportAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_printsupport.pri"
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_printsupport_private.pri"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/pkgconfig/Qt6PrintSupport.pc")
endif()

