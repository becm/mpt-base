# create solver driver library
cmake_minimum_required(VERSION 2.6)

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

set(dirs . client data util loader)
set(headers client.h loader.h)

foreach(dir IN ITEMS ${dirs})
  aux_source_directory(${dir} sources)
endforeach(dir)

include_directories(. ../base/mptcore ../base/mptplot ../solver)

add_library(mptclient SHARED ${sources})
set_target_properties(mptclient PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(mptclient mptplot mptcore dl)

install(TARGETS mptclient DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers}  DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
