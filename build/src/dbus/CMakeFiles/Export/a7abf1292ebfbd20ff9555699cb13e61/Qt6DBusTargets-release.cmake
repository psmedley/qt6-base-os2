#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Qt6::DBus" for configuration "Release"
set_property(TARGET Qt6::DBus APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Qt6::DBus PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "dbus-1"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libQt6DBus.so.6.4.2"
  IMPORTED_SONAME_RELEASE "libQt6DBus.so.6"
  )

list(APPEND _cmake_import_check_targets Qt6::DBus )
list(APPEND _cmake_import_check_files_for_Qt6::DBus "${_IMPORT_PREFIX}/lib/libQt6DBus.so.6.4.2" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
