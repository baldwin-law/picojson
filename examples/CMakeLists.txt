cmake_minimum_required(VERSION 3.10)
project(example)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

############################################## unit test ##############################################
set(EXAMPLE_SRCS 
    example.c
)

add_executable(example ${EXAMPLE_SRCS})
target_link_libraries(example pico_json)