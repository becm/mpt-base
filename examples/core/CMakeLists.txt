# tests for MPT core library
cmake_minimum_required(VERSION 2.6)

add_executable(coding coding.c)
target_include_directories(coding PRIVATE ${Mpt_CORE_INCLUDE_DIRS})
target_link_libraries(coding mptcore)
add_test(NAME coding COMMAND coding "cobs")

add_executable(ident ident.c)
target_include_directories(ident PRIVATE ${Mpt_CORE_INCLUDE_DIRS})
target_link_libraries(ident mptcore)
add_test(NAME ident COMMAND ident "2" "28" "29" "60" "61" "124" "125" "252" "253" "4080")

add_executable(environ environ.c)
target_include_directories(environ PRIVATE ${Mpt_CORE_INCLUDE_DIRS})
target_link_libraries(environ mptcore)
add_test(NAME environ COMMAND environ "mpt_*")

add_executable(config config.c)
target_include_directories(config PRIVATE ${Mpt_CORE_INCLUDE_DIRS})
target_link_libraries(config mptcore)
add_test(NAME config COMMAND config)

add_executable(parse parse.c)
target_include_directories(parse PRIVATE ${Mpt_CORE_INCLUDE_DIRS})
target_link_libraries(parse mptcore)
# see MPT_NAMEFLAG(Flags) in parse.h for restriction configuration
add_test(NAME parse_online COMMAND parse "${CMAKE_CURRENT_SOURCE_DIR}/online.txt" "{*} =;!# `" "Ef")
add_test(NAME parse_layout COMMAND parse "${CMAKE_CURRENT_SOURCE_DIR}/layout.txt" "[*] = " "Esnw")
add_test(NAME parse_layout_alt COMMAND parse "${CMAKE_CURRENT_SOURCE_DIR}/layout.txt" "[*] = !")
add_test(NAME parse_subsect COMMAND parse "${CMAKE_CURRENT_SOURCE_DIR}/subsect.lay" "{*} =;!#" "E")
add_test(NAME parse_config COMMAND parse "${CMAKE_CURRENT_SOURCE_DIR}/config.txt" "[ ] = #" "Esc")
