# tests for MPT C++ library
cmake_minimum_required(VERSION 2.6)

set(EXAMPLES_CXX_INCLUDE ${Mpt_CXX_INCLUDE_DIRS} ${Mpt_CORE_INCLUDE_DIRS})

add_executable(array_cxx array.cpp)
target_include_directories(array_cxx PRIVATE ${EXAMPLES_CXX_INCLUDE})
target_link_libraries(array_cxx mpt++)
add_test(NAME array_cxx COMMAND array_cxx)

add_executable(config_cxx config.cpp)
target_include_directories(config_cxx PRIVATE ${EXAMPLES_CXX_INCLUDE})
target_link_libraries(config_cxx mpt++)
add_test(NAME config_cxx COMMAND config_cxx)

add_executable(queue_cxx queue.cpp)
target_include_directories(queue_cxx PRIVATE ${EXAMPLES_CXX_INCLUDE})
target_link_libraries(queue_cxx mpt++)
add_test(NAME queue_cxx COMMAND queue_cxx)

add_executable(types_cxx types.cpp)
target_include_directories(types_cxx PRIVATE ${EXAMPLES_CXX_INCLUDE})
target_link_libraries(types_cxx mpt++)
add_test(NAME types_cxx COMMAND types_cxx)

add_executable(value_cxx value.cpp)
target_include_directories(value_cxx PRIVATE ${EXAMPLES_CXX_INCLUDE})
target_link_libraries(value_cxx mpt++)
add_test(NAME value_cxx COMMAND value_cxx)

if(MPT_WITH_IO)
  add_executable(buffer_cxx buffer.cpp)
  target_include_directories(buffer_cxx PRIVATE ${EXAMPLES_CXX_INCLUDE})
  target_link_libraries(buffer_cxx mpt++)
  add_test(NAME buffer_cxx COMMAND buffer_cxx)
endif(MPT_WITH_IO)


if(MPT_WITH_PLOT)
  set(EXAMPLES_CXX_PLOT_INCLUDE ${Mpt_PLOT_INCLUDE_DIRS} ${EXAMPLES_CXX_INCLUDE})
  add_executable(object_cxx object.cpp)
  target_include_directories(object_cxx PRIVATE ${EXAMPLES_CXX_PLOT_INCLUDE})
  target_link_libraries(object_cxx mpt++)
  add_test(NAME object_cxx COMMAND object_cxx)
  
  add_executable(iter_cxx iter.cpp)
  target_include_directories(iter_cxx PRIVATE ${EXAMPLES_CXX_PLOT_INCLUDE})
  target_link_libraries(iter_cxx mpt++)
  add_test(NAME iter_cxx COMMAND iter_cxx)
  
  add_executable(map_cxx map.cpp)
  target_include_directories(map_cxx PRIVATE ${EXAMPLES_CXX_PLOT_INCLUDE})
  target_link_libraries(map_cxx mpt++)
  add_test(NAME map_cxx COMMAND map_cxx)
  
  add_executable(layout_cxx layout.cpp)
  target_include_directories(layout_cxx PRIVATE ${EXAMPLES_CXX_PLOT_INCLUDE})
  target_link_libraries(layout_cxx mpt++)
  add_test(NAME layout_cxx COMMAND layout_cxx "${CMAKE_CURRENT_SOURCE_DIR}/layout.lay")
  
  add_executable(value_store_cxx value_store.cpp)
  target_include_directories(value_store_cxx PRIVATE ${EXAMPLES_CXX_PLOT_INCLUDE})
  target_link_libraries(value_store_cxx mpt++)
  add_test(NAME value_store_cxx COMMAND value_store_cxx)
  
  if (MPT_WITH_IO)
    set(EXAMPLES_CXX_IO_PLOT_INCLUDE ${Mpt_IO_INCLUDE_DIRS} ${EXAMPLES_CXX_PLOT_INCLUDE})
    add_executable(output_cxx output.cpp)
    target_include_directories(output_cxx PRIVATE ${EXAMPLES_CXX_IO_PLOT_INCLUDE})
    target_link_libraries(output_cxx mpt++)
    add_test(NAME output_cxx COMMAND output_cxx)
  endif(MPT_WITH_IO)
endif(MPT_WITH_PLOT)
