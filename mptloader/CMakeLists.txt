# create MPT plotting library
cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} .. ../mptcore)

aux_source_directory(. sources)

add_library(mptloader SHARED ${sources} libinfo.c)
set_target_properties(mptloader PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(mptcore)

install(TARGETS mptloader  DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
