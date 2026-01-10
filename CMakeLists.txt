cmake_minimum_required(VERSION 3.10)

project(adsb_anomaly_detection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(adsb_main
    src/main.cpp
)

target_include_directories(adsb_main PRIVATE src)

add_executable(fuzzy_test
    test/TestFuzzySystem.cpp
)

target_include_directories(fuzzy_test PRIVATE src)

