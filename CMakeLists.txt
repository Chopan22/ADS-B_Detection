cmake_minimum_required(VERSION 3.15)
project(AdsbFuzzyGA VERSION 1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Source directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(GA_DIR ${SRC_DIR}/ga)
set(FUZZY_DIR ${SRC_DIR}/fuzzy)
set(ADSB_DIR ${SRC_DIR}/adsb)
set(FEATURES_DIR ${SRC_DIR}/features)
set(PREPROCESSING_DIR ${SRC_DIR}/preprocessing)
set(ANALYSIS_DIR ${SRC_DIR}/analysis)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)

# Include directories
include_directories(${SRC_DIR})

# GA Library
add_library(ga STATIC
    ${GA_DIR}/Chromosome.cpp
    ${GA_DIR}/Selection.cpp
    ${GA_DIR}/Fitness.cpp
    ${GA_DIR}/Population.cpp
    ${GA_DIR}/GAEngine.cpp
)

# Main optimizer executable
add_executable(optimizer
    ${SRC_DIR}/optimizer.cpp
)

target_link_libraries(optimizer PRIVATE ga)

# Unit tests (optional, built with GA_TEST_MODE)
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    add_compile_definitions(GA_TEST_MODE)
    
    add_executable(ga_unit_test
        ${TEST_DIR}/ga_unit_test.cpp
        ${GA_DIR}/Chromosome.cpp
        ${GA_DIR}/Selection.cpp
        ${GA_DIR}/Fitness.cpp
        ${GA_DIR}/Population.cpp
        ${GA_DIR}/GAEngine.cpp
    )
    
    add_executable(fuzzy_ga_integration_test
        ${TEST_DIR}/fuzzy_ga_int_test.cpp
        ${GA_DIR}/Chromosome.cpp
        ${GA_DIR}/Selection.cpp
        ${GA_DIR}/Fitness.cpp
        ${GA_DIR}/Population.cpp
        ${GA_DIR}/GAEngine.cpp
    )
endif()

install(TARGETS optimizer DESTINATION bin)

add_custom_target(run-optimizer
    COMMAND optimizer ${DATA_FILE}
    DEPENDS optimizer
    COMMENT "Running optimizer"
)

add_custom_target(analyze
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/analyze_results.py
    COMMENT "Generating analysis graphs"
)

# Create results directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/results)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/results/graphs)

# Print summary
message(STATUS "")
message(STATUS "========================================")
message(STATUS "ADS-B Fuzzy GA Optimizer Configuration")
message(STATUS "========================================")
message(STATUS "Build type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:    ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:        ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build tests:     ${BUILD_TESTS}")
message(STATUS "Install prefix:  ${CMAKE_INSTALL_PREFIX}")
message(STATUS "========================================")
message(STATUS "")
message(STATUS "Available targets:")
message(STATUS "  optimizer        - Build main optimizer")
message(STATUS "  validator        - Build data validator")
message(STATUS "  run-validator    - Run validator (set DATA_FILE)")
message(STATUS "  run-optimizer    - Run optimizer (set DATA_FILE)")
message(STATUS "  analyze          - Generate analysis graphs")
if(BUILD_TESTS)
    message(STATUS "  ga_unit_test     - Build GA unit tests")
    message(STATUS "  fuzzy_ga_integration_test - Build integration test")
endif()
message(STATUS "")
