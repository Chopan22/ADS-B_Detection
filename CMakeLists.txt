cmake_minimum_required(VERSION 3.10)
project(adsb_anomaly_detection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# GA Library sources
set(GA_SOURCES
    src/ga/Chromosome.cpp
    src/ga/Fitness.cpp
    src/ga/GAEngine.cpp
    src/ga/Population.cpp
    src/ga/Selection.cpp
)

# Main ADSB executable
add_executable(adsb_main
    src/main.cpp
    ${GA_SOURCES}
)

target_include_directories(adsb_main PRIVATE src)

# Fuzzy system test executable
add_executable(fuzzy_test
    test/TestFuzzySystem.cpp
)

target_include_directories(fuzzy_test PRIVATE src)

# GA test executable
add_executable(ga_test
    src/ga/ga_test.cpp
    ${GA_SOURCES}
)

target_include_directories(ga_test PRIVATE src)

add_executable(fuzzy_ga_test
    test/fuzzy_ga_integration_test.cpp
    ${GA_SOURCES}
)

target_include_directories(fuzzy_ga_test PRIVATE src)

# Optional: Add install targets
install(TARGETS adsb_main fuzzy_test ga_test
    RUNTIME DESTINATION bin
)

# Optional: Enable testing
enable_testing()
add_test(NAME FuzzySystemTest COMMAND fuzzy_test)
add_test(NAME GATest COMMAND ga_test)
add_test(NAME FuzzyGAIntegration COMMAND fuzzy_ga_test)
