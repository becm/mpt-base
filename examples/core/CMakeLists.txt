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
