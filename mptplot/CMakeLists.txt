# create MPT plotting library
cmake_minimum_required(VERSION 2.6)

include_directories(../mptcore)

set(subdirs
  layout
  output
  values
)

# submodules have headers
foreach(dir IN ITEMS ${subdirs})
  aux_source_directory(${dir} sources)
  set(headers ${dir}.h ${headers})
endforeach(dir)

aux_source_directory(. sources)
include_directories(${CMAKE_CURRENT_BINARY_DIR} .)

add_library(mptplot SHARED ${sources})
set_target_properties(mptplot PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS mptplot  DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
