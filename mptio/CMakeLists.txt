# create MPT I/O library
cmake_minimum_required(VERSION 2.6)


# to dir sources and submodules
set(sources connect.c init.c output_new.c)
foreach(dir IN ITEMS notify outdata stream)
  aux_source_directory(${dir} sources)
endforeach(dir)

set(headers notify.h stream.h)

add_library(mptio SHARED ${sources} libinfo.c)
set_target_properties(mptio PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(mptio PRIVATE .. . ${Mpt_PLOT_INCLUDE_DIRS} ${Mpt_CORE_INCLUDE_DIRS})
target_link_libraries(mptio ${Mpt_CORE_LIBRARIES})

install(TARGETS mptio    DESTINATION ${MPT_INSTALL_LIB} EXPORT mpt-targets)
install(FILES ${headers} DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)
