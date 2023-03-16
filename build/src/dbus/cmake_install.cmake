# Install script for directory: /home/psmedley/qt6-base-os2/src/dbus

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtDBus" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/include/QtDBus/qtdbusexports.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/dbus/meta_types/qt6dbus_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6DBus/Qt6DBusConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6DBus/Qt6DBusConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6DBus/Qt6DBusConfigVersionImpl.cmake"
    "/home/psmedley/qt6-base-os2/src/dbus/Qt6DBusMacros.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/psmedley/qt6-base-os2/build/lib/libQt6DBus.so.6.4.2"
    "/home/psmedley/qt6-base-os2/build/lib/libQt6DBus.so.6"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so.6"
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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so"
         RPATH "\$ORIGIN")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6DBus.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6DBus.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtDBus/6.4.2/QtDBus/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/dbus/dbus_minimal_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbus_symbols_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusabstractadaptor_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusabstractinterface_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusargument_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusconnection_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusconnectionmanager_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbuscontext_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusintegrator_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusinterface_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusintrospection_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusmessage_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusmetaobject_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusmetatype_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbuspendingcall_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusthreaddebug_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusutil_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusxmlparser_p.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qtdbusglobal_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtDBus" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/qtdbusexports.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusabstractadaptor.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusabstractinterface.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusargument.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusconnection.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusconnectioninterface.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbuscontext.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbuserror.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusextratypes.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusinterface.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusmessage.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusmetatype.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbuspendingcall.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbuspendingreply.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusreply.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusserver.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusservicewatcher.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusunixfiledescriptor.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qdbusvirtualobject.h"
    "/home/psmedley/qt6-base-os2/src/dbus/qtdbusglobal.h"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusAbstractAdaptor"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusAbstractInterfaceBase"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusAbstractInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusArgument"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusConnection"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusConnectionInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusContext"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusError"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusObjectPath"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusSignature"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusVariant"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusMessage"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusMetaType"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusPendingCall"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusPendingCallWatcher"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusPendingReplyBase"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusPendingReply"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusReply"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusServer"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusServiceWatcher"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusUnixFileDescriptor"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QDBusVirtualObject"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/qdbusmacros.h"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/qtdbusversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QtDBusVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QtDBus"
    "/home/psmedley/qt6-base-os2/build/include/QtDBus/QtDBusDepends"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus/Qt6DBusTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus/Qt6DBusTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/dbus/CMakeFiles/Export/a7abf1292ebfbd20ff9555699cb13e61/Qt6DBusTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus/Qt6DBusTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus/Qt6DBusTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/dbus/CMakeFiles/Export/a7abf1292ebfbd20ff9555699cb13e61/Qt6DBusTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/dbus/CMakeFiles/Export/a7abf1292ebfbd20ff9555699cb13e61/Qt6DBusTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus/Qt6DBusVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus/Qt6DBusVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/dbus/CMakeFiles/Export/a7abf1292ebfbd20ff9555699cb13e61/Qt6DBusVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus/Qt6DBusVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus/Qt6DBusVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/dbus/CMakeFiles/Export/a7abf1292ebfbd20ff9555699cb13e61/Qt6DBusVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/DBus.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_html_docs_DBus")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc/qtdbus" TYPE DIRECTORY FILES "/home/psmedley/qt6-base-os2/build/./doc/qtdbus/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_qch_docs_DBus")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./doc/qtdbus.qch")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6DBus" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6DBus/Qt6DBusAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_dbus.pri"
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_dbus_private.pri"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/pkgconfig/Qt6DBus.pc")
endif()

