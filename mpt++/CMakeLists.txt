# create MPT C++ library
cmake_minimum_required(VERSION 2.6)

aux_source_directory(. sources)
include_directories(${CMAKE_CURRENT_BINARY_DIR} . .. ../mptplot ../mptio ../mptcore)
set(headers object.h graphic.h)

add_library(mpt++ SHARED ${sources})
set_target_properties(mpt++ PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(mpt++ mptio mptplot mptcore)

install(TARGETS mpt++    DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
