cmake_minimum_required (VERSION 2.6)
project (fstjson_test C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall")
endif()

add_library(fstjson fstjson.c)
add_executable(fstjson_test test.c)
target_link_libraries(fstjson_test fstjson)
