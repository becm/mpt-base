cmake_minimum_required(VERSION 2.6)

project(mptbase)

# create path-independant libraries
set(CMAKE_INSTALL_RPATH "$ORIGIN/")

# target path options
set(MPT_INSTALL_LIB lib/${CMAKE_LIBRARY_ARCHITECTURE}
    CACHE FILEPATH "runtime libraries path (install prefix prepended if relative)")

set(MPT_INSTALL_INCLUDE include
    CACHE FILEPATH "header files (install prefix prepended if relative)")

if (NOT IS_ABSOLUTE ${MPT_INSTALL_INCLUDE})
  set(MPT_INSTALL_INCLUDE ${CMAKE_INSTALL_PREFIX}/${MPT_INSTALL_INCLUDE})
endif()

if (NOT IS_ABSOLUTE ${MPT_INSTALL_INCLUDE})
  set(MPT_INSTALL_LIB ${CMAKE_INSTALL_PREFIX}/${MPT_INSTALL_LIB})
endif()

# release information
set(MPT_RELEASE "" CACHE STRING "set to non-emty for release info")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/release.h "/* release info */\n")
if (MPT_RELEASE)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/release.h "#define MPT_RELEASE \"${MPT_RELEASE}\"\n")
endif(MPT_RELEASE)

# conditional libraries to build
option(MPT_WITH_CXX "create MPT C++ library (requires plot library)" ON)
if (MPT_WITH_CXX)
  add_subdirectory(mpt++)
endif(MPT_WITH_CXX)

option(MPT_WITH_PLOT "create MPT plotting extension library" ON)
if (MPT_WITH_PLOT)
  add_subdirectory(mptplot)
endif (MPT_WITH_PLOT)

# non-optional basic library
add_subdirectory(mptcore)

set(CPACK_PACKAGE_NAME "mpt-base")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Message Passing/Processing Toolkit")

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Marc Becker") #required

INCLUDE(CPack)
