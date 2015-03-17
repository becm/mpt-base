# create MPT plotting library
cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR} . .. ../mptcore)

set(subdirs
  plot
  output
  values
)

# submodules and headers
foreach(dir IN ITEMS ${subdirs})
  aux_source_directory(${dir} sources)
  set(headers ${dir}.h ${headers})
endforeach(dir)

aux_source_directory(. sources)

add_library(mptplot SHARED ${sources})
set_target_properties(mptplot PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(mptplot mptcore)

install(TARGETS mptplot  DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
