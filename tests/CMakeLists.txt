cmake_minimum_required(VERSION 3.10)
project(test)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

add_library(catch catch.cpp)

############################################## unit test ##############################################
set(TEST_SRCS 
    test_pj.cpp
)

add_executable(unit_test ${TEST_SRCS})
target_link_libraries(unit_test pico_json catch)

include(CTest)
enable_testing()

# Fix the test command to use proper path
add_test(NAME test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unit_test)

# Add coverage targets if coverage is enabled
if(PICO_JSON_ENABLE_COVERAGE)
    # Create a custom target for generating coverage reports
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        # Add target for generating coverage info
        add_custom_target(coverage
            # Initialize coverage counters
            COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters

            # Run the tests directly to ensure coverage is generated
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unit_test
            
            # Small delay to ensure coverage data is written
            COMMAND ${CMAKE_COMMAND} -E sleep 1
            
            # Capture coverage info
            COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/coverage.info
            
            # Filter out system headers and test files
            COMMAND ${LCOV_PATH} --remove ${CMAKE_BINARY_DIR}/coverage.info '/usr/include/*' '*/tests/*' --output-file ${CMAKE_BINARY_DIR}/coverage.info.filtered
            
            # Generate HTML report
            COMMAND ${GENHTML_PATH} --demangle-cpp -o ${CMAKE_BINARY_DIR}/coverage ${CMAKE_BINARY_DIR}/coverage.info.filtered
            
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )
        
        add_custom_command(TARGET coverage POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated at: file://${CMAKE_BINARY_DIR}/coverage/index.html"
        )
    else()
        message(WARNING "Code coverage tools not found (gcov, lcov, or genhtml). Coverage target will not be available.")
    endif()
endif()