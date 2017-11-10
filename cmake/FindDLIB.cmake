INCLUDE(FindPackageHandleStandardArgs)

set(DLIB_INCLUDE_DIR /usr/local/include)
SET(DLIB_LibrarySearchPaths
  /usr/local/lib
  /usr/lib
  )

find_path(DLIB_SUBDIR dlib/CMakeLists.txt
  HINTS "/usr/include" "/usr/local/include"
  )

if(DLIB_SUBDIR)
  message("dlib is installed as pure header files")
  #if dlib is installed like
  add_definitions(-DUSE_AVX_INSTRUCTIONS)
  add_definitions(-DUSE_SSE4_INSTRUCTIONS)
  set(DLIB_INCLUDE_DIR ${DLIB_SUBDIR})
  add_subdirectory("${DLIB_SUBDIR}/dlib" ${CMAKE_BINARY_DIR}/build_dlib)
  set(DLIB_LIBRARY dlib::dlib)
else()
  message("dlib is installed as binary")
  find_library(DLIB_LIBRARY
    NAMES dlib
    PATHS ${DLIB_LibrarySearchPaths}
    )
endif()
# Handle the REQUIRED argument and set the <UPPERCASED_NAME>_FOUND variable
# The package is found if all variables listed are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DLIB "Could NOT find DLIB library"
  DLIB_LIBRARY
  DLIB_INCLUDE_DIR
)

SET(DLIB_LIBRARIES)
LIST(APPEND DLIB_LIBRARIES ${DLIB_LIBRARY})

MARK_AS_ADVANCED(
  DLIB_INCLUDE_DIR
  DLIB_LIBRARIES
  DLIB_FOUND
)
