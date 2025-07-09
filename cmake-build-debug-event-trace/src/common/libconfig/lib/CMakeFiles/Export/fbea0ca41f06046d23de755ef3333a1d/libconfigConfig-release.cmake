#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "libconfig::config" for configuration "Release"
set_property(TARGET libconfig::config APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libconfig::config PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libconfig.a"
  )

list(APPEND _cmake_import_check_targets libconfig::config )
list(APPEND _cmake_import_check_files_for_libconfig::config "${_IMPORT_PREFIX}/lib/libconfig.a" )

# Import target "libconfig::config++" for configuration "Release"
set_property(TARGET libconfig::config++ APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libconfig::config++ PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libconfig++.a"
  )

list(APPEND _cmake_import_check_targets libconfig::config++ )
list(APPEND _cmake_import_check_files_for_libconfig::config++ "${_IMPORT_PREFIX}/lib/libconfig++.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
