# create MPT I/O library
cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} . .. ../mptcore ../mptplot)

# submodules and headers
foreach(dir IN ITEMS outdata notify stream)
  aux_source_directory(${dir} sources)
endforeach(dir)
set(headers notify.h stream.h)

set(sources ${sources} connect.c init.c output_new.c)

add_library(mptio SHARED ${sources} libinfo.c)
set_target_properties(mptio PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(mptio mptcore dl)

install(TARGETS mptio    DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
