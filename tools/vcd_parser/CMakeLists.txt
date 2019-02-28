cmake_minimum_required(VERSION 2.8)
project(vcd_parser CXX)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} -O3 -std=c++17")

add_executable(vcd_parser src/main.cpp src/vcd_header.cpp src/vcd_data.cpp)
