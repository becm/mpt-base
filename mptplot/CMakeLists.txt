# create MPT plotting library
cmake_minimum_required(VERSION 2.6)

# subdirectory sources
set(sources
  mapping.c
  rawdata_create.c
  rawdata_type_traits.c
)
foreach(dir IN ITEMS history layout output values)
  aux_source_directory(${dir} sources)
endforeach(dir)

set(headers history.h layout.h values.h)

add_library(mptplot SHARED ${sources} libinfo.c)
set_target_properties(mptplot PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(mptplot PRIVATE . ${Mpt_CORE_INCLUDE_DIRS})
target_link_libraries(mptplot ${Mpt_CORE_LIBRARIES})

install(TARGETS mptplot  DESTINATION ${MPT_INSTALL_LIB} EXPORT mpt-targets)
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
