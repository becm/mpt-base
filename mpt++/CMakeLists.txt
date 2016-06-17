# create MPT C++ library
cmake_minimum_required(VERSION 2.6)

set(sources
  array.cpp config.cpp client.cpp
  logger.cpp
  message.cpp meta.cpp meta_new.cpp meta_buffer.cpp
  node_new.cpp
  object.cpp parse.cpp queue.cpp
  reference_wrap.cpp value.cpp
)
set(linklibs ${Mpt_CORE_LIBRARIES})
set(includedirs ${Mpt_CORE_INCLUDE_DIRS})

if(MPT_WITH_PLOT)
  set(sources ${sources}
    cycle.cpp graphic.cpp item.cpp layout.cpp
    mapping.cpp polyline.cpp transform.cpp
  )
  list(APPEND includedirs ${Mpt_PLOT_INCLUDE_DIRS})
  list(INSERT linklibs 0 ${Mpt_PLOT_LIBRARIES})
endif(MPT_WITH_PLOT)

if(MPT_WITH_IO)
  set(sources ${sources}
    notify.cpp stream.cpp
  )
  list(APPEND includedirs ${Mpt_IO_INCLUDE_DIRS})
  list(INSERT linklibs 0 ${Mpt_IO_LIBRARIES})
endif(MPT_WITH_IO)

set(headers object.h graphic.h)

add_library(mpt++ SHARED ${sources} ${shared} libinfo.c)
set_target_properties(mpt++ PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(mpt++ PRIVATE .. ${includedirs})
target_link_libraries(mpt++ ${linklibs})

if (${CMAKE_VERSION} VERSION_LESS 3.1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  target_compile_features(mpt++ PUBLIC cxx_decltype cxx_range_for)
endif()

install(TARGETS mpt++    DESTINATION ${MPT_INSTALL_LIB} EXPORT mpt-targets)
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
