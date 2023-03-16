# Install script for directory: /home/psmedley/qt6-base-os2/src/sql

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtSql" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/include/QtSql/qtsqlexports.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtSql" TYPE FILE OPTIONAL RENAME "qtsql-config.h" FILES "/home/psmedley/qt6-base-os2/build/src/sql/qtsql-config.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtSql/6.4.2/QtSql/private" TYPE FILE OPTIONAL RENAME "qtsql-config_p.h" FILES "/home/psmedley/qt6-base-os2/build/src/sql/qtsql-config_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/sql/meta_types/qt6sql_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Sql/Qt6SqlConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Sql/Qt6SqlConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Sql/Qt6SqlConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/psmedley/qt6-base-os2/build/lib/libQt6Sql.so.6.4.2"
    "/home/psmedley/qt6-base-os2/build/lib/libQt6Sql.so.6"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so.6"
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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so"
         RPATH "\$ORIGIN")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6Sql.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Sql.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtSql/6.4.2/QtSql/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqlcachedresult_p.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqldriver_p.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqlnulldriver_p.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqlresult_p.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qtsqlglobal_p.h"
    "/home/psmedley/qt6-base-os2/src/sql/models/qsqlquerymodel_p.h"
    "/home/psmedley/qt6-base-os2/src/sql/models/qsqltablemodel_p.h"
    "/home/psmedley/qt6-base-os2/build/src/sql/qtsql-config_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtSql" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtSql/qtsqlexports.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqldatabase.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqldriver.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqldriverplugin.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqlerror.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqlfield.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqlindex.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqlquery.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqlrecord.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qsqlresult.h"
    "/home/psmedley/qt6-base-os2/src/sql/kernel/qtsqlglobal.h"
    "/home/psmedley/qt6-base-os2/src/sql/models/qsqlquerymodel.h"
    "/home/psmedley/qt6-base-os2/src/sql/models/qsqlrelationaldelegate.h"
    "/home/psmedley/qt6-base-os2/src/sql/models/qsqlrelationaltablemodel.h"
    "/home/psmedley/qt6-base-os2/src/sql/models/qsqltablemodel.h"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlDriverCreatorBase"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlDriverCreator"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlDatabase"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlDriver"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlDriverPlugin"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlError"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlField"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlIndex"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlQuery"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlRecord"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlResult"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSql"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlQueryModel"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlRelationalDelegate"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlRelation"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlRelationalTableModel"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QSqlTableModel"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/qsql.h"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/qtsqlversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QtSqlVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QtSql"
    "/home/psmedley/qt6-base-os2/build/include/QtSql/QtSqlDepends"
    "/home/psmedley/qt6-base-os2/build/src/sql/qtsql-config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql/Qt6SqlTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql/Qt6SqlTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/sql/CMakeFiles/Export/6daf95e385d7cc8ad37a0af677fcce5d/Qt6SqlTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql/Qt6SqlTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql/Qt6SqlTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/sql/CMakeFiles/Export/6daf95e385d7cc8ad37a0af677fcce5d/Qt6SqlTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/sql/CMakeFiles/Export/6daf95e385d7cc8ad37a0af677fcce5d/Qt6SqlTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql/Qt6SqlVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql/Qt6SqlVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/sql/CMakeFiles/Export/6daf95e385d7cc8ad37a0af677fcce5d/Qt6SqlVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql/Qt6SqlVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql/Qt6SqlVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/sql/CMakeFiles/Export/6daf95e385d7cc8ad37a0af677fcce5d/Qt6SqlVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/Sql.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_html_docs_Sql")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc/qtsql" TYPE DIRECTORY FILES "/home/psmedley/qt6-base-os2/build/./doc/qtsql/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_qch_docs_Sql")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./doc/qtsql.qch")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Sql" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Sql/Qt6SqlAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_sql.pri"
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_sql_private.pri"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/pkgconfig/Qt6Sql.pc")
endif()

