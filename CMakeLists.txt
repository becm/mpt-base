# create basic mpt resources
cmake_minimum_required(VERSION 2.6)

project(mptbase)

# create path-independant libraries
include(mpt-config.cmake)

# non-optional basic library
add_subdirectory(mptcore)

# release information
set(MPT_RELEASE "" CACHE STRING "set to non-emty for release info")
if (MPT_RELEASE)
  file(WRITE ${CMAKE_BINARY_DIR}/version.h "/* release info */
#define RELEASE_BUILD \"${MPT_RELEASE}\"
#include \"${CMAKE_SOURCE_DIR}/version.h\"
")
endif(MPT_RELEASE)

# conditional libraries to build
option(MPT_WITH_LOADER "create MPT loader library" ON)
if (MPT_WITH_LOADER)
  add_subdirectory(mptloader)
endif (MPT_WITH_LOADER)

option(MPT_WITH_IO "create MPT I/O extension library" ON)
if (MPT_WITH_IO)
  add_subdirectory(mptio)
endif (MPT_WITH_IO)

option(MPT_WITH_PLOT "create MPT plotting extension library" ON)
if (MPT_WITH_PLOT)
  add_subdirectory(mptplot)
endif (MPT_WITH_PLOT)

option(MPT_WITH_CXX "create MPT C++ library" ON)
if (MPT_WITH_CXX)
  add_subdirectory(mpt++)
endif(MPT_WITH_CXX)

set(CPACK_PACKAGE_NAME "mpt-base")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Message Passing/Processing Toolkit")

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Marc Becker") #required

INCLUDE(CPack)
