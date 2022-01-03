# create MPT examples
cmake_minimum_required(VERSION 2.6)

add_compile_options("-DMPT_INCLUDE(x)=#x")

add_subdirectory(core)

if(MPT_WITH_CXX)
  add_subdirectory(cxx)
endif(MPT_WITH_CXX)

if(MPT_WITH_CXX)
  if(MPT_WITH_IO)
    add_executable(client client.cpp)
    target_include_directories(client PRIVATE ${Mpt_IO_INCLUDE_DIRS} ${Mpt_CORE_INCLUDE_DIRS})
    target_link_libraries(client mpt++)
    add_test(NAME client COMMAND client)
  endif(MPT_WITH_IO)
endif(MPT_WITH_CXX)

if(MPT_WITH_PLOT)
  set(EXAMPLES_PLOT_INCLUDE ${Mpt_PLOT_INCLUDE_DIRS} ${Mpt_CORE_INCLUDE_DIRS})
  add_executable(axis axis.c)
  target_include_directories(axis PRIVATE ${EXAMPLES_PLOT_INCLUDE})
  target_link_libraries(axis mptplot)
  add_test(NAME axis COMMAND axis "title=my axis" "intervals=3" "tpos=Up")
  
  add_executable(iter iter.c)
  target_include_directories(iter PRIVATE ${EXAMPLES_PLOT_INCLUDE})
  target_link_libraries(iter mptplot)
  add_test(NAME iter COMMAND iter "1" "fact(3:2e-2::1)" "lin(4 : 1 2)")
  
  add_executable(gen gen.c)
  target_include_directories(gen PRIVATE ${EXAMPLES_PLOT_INCLUDE})
  target_link_libraries(gen mptplot)
  add_test(NAME gen COMMAND gen "9" "poly 1 0 0 : -4" "lin 1 0")
  
  if(MPT_WITH_CXX)
    set(EXAMPLES_PLOT_CXX_INCLUDE ${Mpt_CXX_INCLUDE_DIRS} ${EXAMPLES_PLOT_INCLUDE})
    add_executable(graph graph.cpp)
    target_include_directories(graph PRIVATE ${EXAMPLES_PLOT_CXX_INCLUDE})
    target_link_libraries(graph mpt++)
    add_test(NAME graph COMMAND graph "w1 w2" "ax")
    
    add_executable(graphic graphic.cpp)
    target_include_directories(graphic PRIVATE ${EXAMPLES_PLOT_CXX_INCLUDE})
    target_link_libraries(graphic mpt++)
    add_test(NAME graphic COMMAND graphic)
  endif(MPT_WITH_CXX)
endif(MPT_WITH_PLOT)


if(MPT_WITH_IO)
  add_executable(pipe io/pipe.c)
  target_include_directories(pipe PRIVATE ${Mpt_IO_INCLUDE_DIRS} ${Mpt_CORE_INCLUDE_DIRS})
  target_link_libraries(pipe mptio)
  add_test(NAME pipe COMMAND pipe "/bin/ls" "..")
endif(MPT_WITH_IO)
