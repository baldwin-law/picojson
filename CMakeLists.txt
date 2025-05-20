cmake_minimum_required(VERSION 3.10.0)
project(pico_json VERSION 0.1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Debug)

option(PICO_JSON_BUILD_TESTS "Build tests" ON)
option(PICO_JSON_BUILD_EXAMPLES "Build examples" ON)
option(PICO_JSON_ENABLE_COVERAGE "Enable coverage reporting" OFF)

# Add coverage flags if enabled
if(PICO_JSON_ENABLE_COVERAGE)
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
  endif()
endif()

add_library(pico_json src/pico_json.c)
target_include_directories(pico_json
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (PICO_JSON_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (PICO_JSON_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
