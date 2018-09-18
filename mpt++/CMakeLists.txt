# create MPT C++ library
cmake_minimum_required(VERSION 2.6)

set(sources
  array.cpp
  client.cpp
  config.cpp
  event.cpp
  identifier.cpp
  io_buffer.cpp io_queue.cpp
  logger.cpp
  makeid.cpp
  message.cpp
  meta_basic.cpp meta_buffer.cpp meta_create.cpp meta_new.cpp meta_typed.cpp
  node_new.cpp
  node.cpp
  object.cpp
  refcount_wrap.cpp
  parse.cpp
  queue.cpp
  refcount_wrap.cpp
  std_cout.cpp
  value_store.cpp
  value.cpp
)
set(linklibs ${Mpt_CORE_LIBRARIES})
set(includedirs ${Mpt_CORE_INCLUDE_DIRS})
set(headers)

if(MPT_WITH_PLOT)
  set(sources ${sources}
    cycle.cpp destination.cpp
    graph.cpp graphic.cpp item.cpp linepart.cpp layout.cpp
    mapping.cpp polyline.cpp transform.cpp
  )
  list(APPEND includedirs ${Mpt_PLOT_INCLUDE_DIRS})
  list(INSERT linklibs 0 ${Mpt_PLOT_LIBRARIES})
  list(APPEND headers graphic.h)
endif(MPT_WITH_PLOT)

if(MPT_WITH_IO)
  set(sources ${sources}
    io.cpp io_stream.cpp
    notify.cpp socket.cpp stream.cpp
  )
  list(APPEND includedirs ${Mpt_IO_INCLUDE_DIRS})
  list(INSERT linklibs 0 ${Mpt_IO_LIBRARIES})
  list(APPEND headers io.h)
endif(MPT_WITH_IO)

add_library(mpt++ SHARED ${sources} ${shared} libinfo.c)
set_target_properties(mpt++ PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(mpt++ PRIVATE .. ${includedirs})
target_link_libraries(mpt++ ${linklibs})

if (${CMAKE_VERSION} VERSION_LESS 3.1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  target_compile_features(mpt++ PUBLIC cxx_decltype cxx_range_for)
endif()

install(TARGETS mpt++ EXPORT mpt-targets DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
