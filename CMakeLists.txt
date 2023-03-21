cmake_minimum_required(VERSION 3.25.2)

project(faf)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)

SET(CMAKE_SOURCE_DIR ../src/)

file(DOWNLOAD
    https://raw.githubusercontent.com/nlohmann/json/master/single_include/nlohmann/json.hpp
    ${CMAKE_SOURCE_DIR}/external/nlohmann/json.hpp
)

file(DOWNLOAD
    https://raw.githubusercontent.com/p-ranav/indicators/v2.3/single_include/indicators/indicators.hpp
    ${CMAKE_SOURCE_DIR}/external/p-ranav/indicators.hpp
)

add_executable(faf src/main.cpp src/util.cpp src/couriers/google.cpp src/couriers/common.cpp)
target_link_libraries(faf curl)

install(TARGETS faf CONFIGURATIONS Release)
