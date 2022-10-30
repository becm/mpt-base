# create MPT core library
cmake_minimum_required(VERSION 2.6)

set(headers core.h)

set(subdirs
  array
  client
  config
  convert
  event
  message
  meta
  node
  object
  output
  parse
  queue
  types
)

# submodules have headers
foreach(dir IN ITEMS ${subdirs})
  aux_source_directory(${dir} sources)
  set(headers ${dir}.h ${headers})
endforeach(dir)

# operations defined in core.h
aux_source_directory(misc sources)

add_library(mptcore SHARED ${sources} libinfo.c)
set_target_properties(mptcore PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(mptcore PRIVATE .)

install(TARGETS mptcore EXPORT mpt-targets DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
