# create solver driver library
cmake_minimum_required(VERSION 2.6)

project(mptclient)

include_directories(${CMAKE_CURRENT_BINARY_DIR} . .. ../mptcore ../mptplot)

set(dirs . event loader)
set(headers client.h)

foreach(dir IN ITEMS ${dirs})
  aux_source_directory(${dir} sources)
endforeach(dir)

add_library(mptclient SHARED ${sources})
set_target_properties(mptclient PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(mptclient mptplot mptcore dl)

install(TARGETS mptclient DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers}  DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
