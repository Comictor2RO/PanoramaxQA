#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "c2pa_cpp::c2pa_cpp" for configuration ""
set_property(TARGET c2pa_cpp::c2pa_cpp APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(c2pa_cpp::c2pa_cpp PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libc2pa_cpp.so"
  IMPORTED_SONAME_NOCONFIG "libc2pa_cpp.so"
  )

list(APPEND _cmake_import_check_targets c2pa_cpp::c2pa_cpp )
list(APPEND _cmake_import_check_files_for_c2pa_cpp::c2pa_cpp "${_IMPORT_PREFIX}/lib/libc2pa_cpp.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
