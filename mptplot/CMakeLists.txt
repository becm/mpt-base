# create MPT plotting library
cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} . .. ../mptcore)

# subdirectory sources
foreach(dir IN ITEMS layout output values)
  aux_source_directory(${dir} sources)
  set(headers ${dir}.h ${headers})
endforeach(dir)


add_library(mptplot SHARED ${sources} libinfo.c)
set_target_properties(mptplot PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(mptplot mptcore)

install(TARGETS mptplot  DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
