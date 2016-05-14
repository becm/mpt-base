# create MPT plotting library
cmake_minimum_required(VERSION 2.6)

aux_source_directory(. sources)

set(linklibs ${Mpt_CORE_LIBRARIES})
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  list(APPEND linklibs "dl")
endif()

add_library(mptloader SHARED ${sources} libinfo.c)
set_target_properties(mptloader PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(mptloader PRIVATE .. ${Mpt_CORE_INCLUDE_DIRS})
target_link_libraries(mptloader ${linklibs})

install(TARGETS mptloader  DESTINATION ${MPT_INSTALL_LIB} EXPORT mpt-targets)
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
