# create MPT core library
cmake_minimum_required(VERSION 2.6)

set(headers core.h)

set(subdirs
  array
  config
  event
  message
  node
  parse
  queue
  convert
  client
)

# submodules have headers
foreach(dir IN ITEMS ${subdirs})
  aux_source_directory(${dir} sources)
  set(headers ${dir}.h ${headers})
endforeach(dir)

# metatype operations defined in core.h
aux_source_directory(meta sources)

set(sources ${sources} abort.c identifier.c log.c mode_parse.c reference.c)

add_library(mptcore SHARED ${sources} libinfo.c)
set_target_properties(mptcore PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(mptcore PRIVATE .. . ${Mpt_PLOT_INCLUDE_DIRS} ${Mpt_IO_INCLUDE_DIRS})

install(TARGETS mptcore  DESTINATION ${MPT_INSTALL_LIB} EXPORT mpt-targets)
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
