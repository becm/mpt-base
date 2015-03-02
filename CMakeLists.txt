cmake_minimum_required(VERSION 2.6)

project(mptbase)

# create path-independant libraries
include(mpt-config.cmake)

# conditional libraries to build
option(MPT_WITH_CXX "create MPT C++ library (requires plot library)" ON)
if (MPT_WITH_CXX)
  add_subdirectory(mpt++)
endif(MPT_WITH_CXX)

option(MPT_WITH_CLIENT "create MPT client extension library" ON)
if (MPT_WITH_CLIENT)
  add_subdirectory(mptclient)
endif (MPT_WITH_CLIENT)

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
