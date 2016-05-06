# create MPT C++ library
cmake_minimum_required(VERSION 2.6)

set(sources
  array.cpp config.cpp message.cpp value.cpp logger.cpp
  meta.cpp object.cpp parse.cpp queue.cpp reference.cpp
)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} . .. ../mptcore)
set(linklibs mptcore)

if(MPT_WITH_PLOT)
  set(sources ${sources}
    cycle.cpp graphic.cpp item.cpp mapping.cpp
    layout.cpp polyline.cpp transform.cpp
  )
  include_directories(../mptplot)
  set(linklibs mptplot ${linklibs})
endif(MPT_WITH_PLOT)

if(MPT_WITH_IO)
  set(sources ${sources}
    notify.cpp stream.cpp
  )
  include_directories(../mptio)
  set(linklibs mptio ${linklibs})
endif(MPT_WITH_IO)

set(headers object.h graphic.h)

add_library(mpt++ SHARED ${sources} libinfo.c)
set_target_properties(mpt++ PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(mpt++ ${linklibs})

if (${CMAKE_VERSION} VERSION_LESS 3.1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  target_compile_features(mpt++ PUBLIC cxx_decltype cxx_range_for)
endif()

install(TARGETS mpt++    DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
