# mpt-config.cmake: basic mpt setup

# create path-independant libraries
set(CMAKE_INSTALL_RPATH "$ORIGIN/")

# release information
set(MPT_RELEASE "" CACHE STRING "set to non-emty for release info")
if (MPT_RELEASE)
  add_definitions(-DRELEASE_BUILD="${MPT_RELEASE}")
endif(MPT_RELEASE)

# target path options
set(MPT_INSTALL_LIB lib/${CMAKE_LIBRARY_ARCHITECTURE}
    CACHE FILEPATH "runtime libraries path (install prefix prepended if relative)")
if (NOT IS_ABSOLUTE ${MPT_INSTALL_LIB})
  set(MPT_INSTALL_LIB ${CMAKE_INSTALL_PREFIX}/${MPT_INSTALL_LIB})
endif()

set(MPT_INSTALL_INCLUDE include
    CACHE FILEPATH "header files (install prefix prepended if relative)")
if (NOT IS_ABSOLUTE ${MPT_INSTALL_INCLUDE})
  set(MPT_INSTALL_INCLUDE ${CMAKE_INSTALL_PREFIX}/${MPT_INSTALL_INCLUDE})
endif()
