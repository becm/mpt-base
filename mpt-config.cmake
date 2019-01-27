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


# version and libinfo headers
set(Mpt_INCLUDE_DIRS "${Mpt_DIR}")

# available modules
set(modules "core")
list(APPEND modules "io")
list(APPEND modules "plot")
list(APPEND modules "loader")
list(APPEND modules "cxx")

# need rebuild of module library
if(NOT "${MPT_INSTALL_LIB}" STREQUAL "${libdir}")
  if(NOT libdir)
    message(STATUS "build cache -> ${MPT_INSTALL_LIB}")
  else()
    message(STATUS "rebuild cache -> ${MPT_INSTALL_LIB}")
  endif()
  set(libdir "${MPT_INSTALL_LIB}" CACHE INTERNAL "save library location")
  
  foreach(_mod ${modules})
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
      find_library(Mpt_${_up} NAMES mpt${_comp} PATHS "${MPT_INSTALL_LIB}" NO_DEFAULT_PATH DOC "mpt ${_mod} library")
    endif()
    # local build definition
    if(NOT Mpt_${_up})
      find_path(Mpt_${_up} "CMakeLists.txt" PATH "${CMAKE_CURRENT_LIST_DIR}/mpt${_comp}" NO_DEFAULT_PATH)
    endif()
    # remote build definition
    if(NOT Mpt_${_up})
      find_path(Mpt_${_up} "CMakeLists.txt" PATH "${Mpt_DIR}/mpt${_comp}" NO_DEFAULT_PATH)
      set(Mpt_${_up}_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/mpt/${_mod}" CACHE PATH "build directory for mpt${_comp}")
    endif()
    
    # clear missing entries
    if(NOT Mpt_${_up})
      unset(Mpt_${_up} CACHE)
      unset(Mpt_${_up}_BUILD_DIR CACHE)
      list(REMOVE_ITEM modules "${_mod}")
      message(WARNING "no definition for component ${_mod}")
    endif()
  endforeach()
endif()

# definitions for modules
foreach(_comp ${modules})
  string(TOUPPER ${_comp} _up)
  
  # use created module
  if(IS_DIRECTORY "${Mpt_${_up}}")
    set(Mpt_${_up}_INCLUDE_DIRS "${Mpt_${_up}}")
    set(Mpt_${_up}_LIBRARIES "mpt${_comp}")
  # use existing library
  else()
    set(Mpt_${_up}_INCLUDE_DIRS "${MPT_INSTALL_INCLUDE}/mpt")
    set(Mpt_${_up}_LIBRARIES "${Mpt_${_up}}")
  endif()
endforeach()

# module target/library and definitions
macro(mpt_module _comp)
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
    list(INSERT Mpt_INCLUDE_DIRS 0 "${Mpt_${_up}_INCLUDE_DIRS}")
  else()
    message(STATUS "${_comp} lib: ${Mpt_${_up}}")
    list(APPEND Mpt_INCLUDE_DIRS "${MPT_INSTALL_INCLUDE}/mpt")
  endif()
endmacro()

# normalize component names
unset(components)
foreach(_comp ${Mpt_FIND_COMPONENTS})
  string(TOLOWER ${_comp} _comp)
  list(APPEND components ${_comp})
endforeach()

if(components)
  # all components need core module
  mpt_module("core")
  list(FIND components "core" _pos)
  if (NOT _pos LESS 0)
    list(REMOVE_ITEM components "core")
    list(INSERT Mpt_INCLUDE_DIRS 0 "${Mpt_CORE_INCLUDE_DIRS}")
    list(INSERT Mpt_LIBRARIES 0 "${Mpt_CORE_LIBRARIES}")
  endif()
  # find conditional modules in correct order
  foreach(_mod "loader" "plot" "io" "cxx")
    list(FIND components ${_mod} _pos)
    list(REMOVE_ITEM components ${_mod})
    if (NOT _pos LESS 0)
      mpt_module(${_mod})
      string(TOUPPER ${_mod} _mod)
      list(INSERT Mpt_LIBRARIES 0 "${Mpt_${_mod}_LIBRARIES}")
      if(IS_DIRECTORY "${Mpt_${_mod}}")
        list(INSERT Mpt_INCLUDE_DIRS 0 "${Mpt_${_mod}_INCLUDE_DIRS}")
      else()
        list(APPEND Mpt_INCLUDE_DIRS "${Mpt_${_mod}_INCLUDE_DIRS}")
      endif()
    endif()
  endforeach(_mod)
endif(components)

if(components)
  message(FATAL_ERROR "unknown components: ${components}")
endif(components)
