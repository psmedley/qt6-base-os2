# Install script for directory: /home/psmedley/qt6-base-os2/src/xml

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtXml" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/include/QtXml/qtxmlexports.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtXml" TYPE FILE OPTIONAL RENAME "qtxml-config.h" FILES "/home/psmedley/qt6-base-os2/build/src/xml/qtxml-config.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtXml/6.4.2/QtXml/private" TYPE FILE OPTIONAL RENAME "qtxml-config_p.h" FILES "/home/psmedley/qt6-base-os2/build/src/xml/qtxml-config_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/xml/meta_types/qt6xml_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Xml/Qt6XmlConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Xml/Qt6XmlConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Xml/Qt6XmlConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/psmedley/qt6-base-os2/build/lib/libQt6Xml.so.6.4.2"
    "/home/psmedley/qt6-base-os2/build/lib/libQt6Xml.so.6"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so.6"
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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so"
         RPATH "\$ORIGIN")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6Xml.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Xml.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtXml/6.4.2/QtXml/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/xml/dom/qdom_p.h"
    "/home/psmedley/qt6-base-os2/src/xml/dom/qdomhelpers_p.h"
    "/home/psmedley/qt6-base-os2/build/src/xml/qtxml-config_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtXml" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtXml/qtxmlexports.h"
    "/home/psmedley/qt6-base-os2/src/xml/qtxmlglobal.h"
    "/home/psmedley/qt6-base-os2/src/xml/dom/qdom.h"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomImplementation"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomNode"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomNodeList"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomDocumentType"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomDocument"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomNamedNodeMap"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomDocumentFragment"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomCharacterData"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomAttr"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomElement"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomText"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomComment"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomCDATASection"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomNotation"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomEntity"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomEntityReference"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QDomProcessingInstruction"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/qtxmlversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QtXmlVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QtXml"
    "/home/psmedley/qt6-base-os2/build/include/QtXml/QtXmlDepends"
    "/home/psmedley/qt6-base-os2/build/src/xml/qtxml-config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml/Qt6XmlTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml/Qt6XmlTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/xml/CMakeFiles/Export/4cd76859c102dee1ced1e953540be7cb/Qt6XmlTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml/Qt6XmlTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml/Qt6XmlTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/xml/CMakeFiles/Export/4cd76859c102dee1ced1e953540be7cb/Qt6XmlTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/xml/CMakeFiles/Export/4cd76859c102dee1ced1e953540be7cb/Qt6XmlTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml/Qt6XmlVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml/Qt6XmlVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/xml/CMakeFiles/Export/4cd76859c102dee1ced1e953540be7cb/Qt6XmlVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml/Qt6XmlVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml/Qt6XmlVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/xml/CMakeFiles/Export/4cd76859c102dee1ced1e953540be7cb/Qt6XmlVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/Xml.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_html_docs_Xml")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc/qtxml" TYPE DIRECTORY FILES "/home/psmedley/qt6-base-os2/build/./doc/qtxml/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_qch_docs_Xml")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./doc/qtxml.qch")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Xml" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Xml/Qt6XmlAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_xml.pri"
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_xml_private.pri"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/pkgconfig/Qt6Xml.pc")
endif()

