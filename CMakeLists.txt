# create basic mpt resources
cmake_minimum_required(VERSION 2.6)

project(mptbase)

# non-optional basic library
set(components core)

# conditional libraries to build
option(MPT_WITH_LOADER "create MPT loader library" ON)
if (MPT_WITH_LOADER)
  list(APPEND components loader)
endif (MPT_WITH_LOADER)

option(MPT_WITH_IO "create MPT I/O extension library" ON)
if (MPT_WITH_IO)
  list(APPEND components io)
endif (MPT_WITH_IO)

option(MPT_WITH_PLOT "create MPT plotting extension library" ON)
if (MPT_WITH_PLOT)
  list(APPEND components plot)
endif (MPT_WITH_PLOT)

option(MPT_WITH_CXX "create MPT C++ library" ON)
if (MPT_WITH_CXX)
  list(APPEND components cxx)
endif(MPT_WITH_CXX)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ".")
find_package(Mpt REQUIRED COMPONENTS ${components})

set(CPACK_PACKAGE_NAME "mpt-base")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Message Passing/Processing Toolkit")

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Marc Becker") #required

INCLUDE(CPack)
