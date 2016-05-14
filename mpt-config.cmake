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

set(Mpt_INCLUDE_DIRS "${Mpt_DIR}")

foreach(_mod ${Mpt_FIND_COMPONENTS})
  string(TOLOWER ${_mod} _mod)
  set(_comp ${_mod})
  string(TOUPPER ${_mod} _up)

  # clean names
  if(${_comp} STREQUAL "cxx")
    set(_comp "++")
  endif()
  
  # clear cache entries
  unset(Mpt_${_up} CACHE)
  unset(Mpt_${_up}_BUILD_DIR CACHE)
  
  # library in target location
  if(NOT Mpt_${_up})
     find_library(Mpt_${_up} NAMES mpt${_comp} PATHS "${MPT_INSTALL_LIB}" NO_DEFAULT_PATH DOC "mpt ${_comp} library")
  endif()
  # local build definition
  if(NOT Mpt_${_up})
    find_path(Mpt_${_up} "CMakeLists.txt" PATH "${CMAKE_CURRENT_SOURCE_DIR}/mpt${_comp}" NO_DEFAULT_PATH)
  endif()
  # remote build definition
  if(NOT Mpt_${_up})
    find_path(Mpt_${_up} "CMakeLists.txt" PATH "${Mpt_DIR}/mpt${_comp}" NO_DEFAULT_PATH)
    set(Mpt_${_up}_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/mpt/${_comp}" CACHE PATH "build directory for mpt${_comp}")
  endif()
  
  if(NOT Mpt_${_up})
    message(FATAL_ERROR "no definition for component ${_comp}")
  endif()
  
endforeach()

foreach(_comp ${Mpt_FIND_COMPONENTS})
  string(TOLOWER ${_comp} _comp)
  string(TOUPPER ${_comp} _up)
  
  # create library
  if(IS_DIRECTORY "${Mpt_${_up}}")
    if (Mpt_${_up}_BUILD_DIR)
      message(STATUS "${_comp}: ${Mpt_${_up}} -> ${Mpt_${_up}_BUILD_DIR}")
      add_subdirectory(${Mpt_${_up}} "${Mpt_${_up}_BUILD_DIR}")
    else()
      message(STATUS "${_comp}: ${Mpt_${_up}}")
      add_subdirectory(${Mpt_${_up}})
    endif()
    set(Mpt_${_up}_INCLUDE_DIRS "${Mpt_${_up}}")
    list(INSERT Mpt_INCLUDE_DIRS 0 "${Mpt_${_up}_INCLUDE_DIRS}")
    set(Mpt_${_up}_LIBRARIES "mpt${_comp}")
  # use existing library
  else()
    message(STATUS "${_comp} lib: ${Mpt_${_up}}")
    set(Mpt_${_up}_INCLUDE_DIRS "${MPT_INSTALL_INCLUDE}/mpt")
    list(APPEND Mpt_INCLUDE_DIRS "${MPT_INSTALL_INCLUDE}/mpt")
    set(Mpt_${_up}_LIBRARIES "${Mpt_${_up}}")
  endif()
  list(INSERT Mpt_LIBRARIES 0 "${Mpt_${_up}_LIBRARIES}")
endforeach()
