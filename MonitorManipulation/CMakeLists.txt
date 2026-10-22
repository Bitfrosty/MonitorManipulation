cmake_minimum_required(VERSION 3.20)
cmake_minimum_required(VERSION 3.20)

project(MonitorManipulation VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(MonitorManipulation
    MonitorManipulation.cpp
)

target_include_directories(MonitorManipulation PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
