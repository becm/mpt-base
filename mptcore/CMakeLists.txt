# create MPT core library
cmake_minimum_required(VERSION 2.6)

set(headers core.h meta.h object.h output.h)

set(subdirs
  array
  client
  config
  convert
  event
  message
  node
  parse
  queue
)

# submodules have headers
foreach(dir IN ITEMS ${subdirs})
  aux_source_directory(${dir} sources)
  set(headers ${dir}.h ${headers})
endforeach(dir)

# metatype operations defined in core.h
aux_source_directory(misc sources)
aux_source_directory(log sources)

add_library(mptcore SHARED ${sources} libinfo.c)
set_target_properties(mptcore PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(mptcore PRIVATE .. .)

install(TARGETS mptcore  DESTINATION ${MPT_INSTALL_LIB} EXPORT mpt-targets)
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
