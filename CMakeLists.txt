cmake_minimum_required(VERSION 3.17)
project(influx-cpp VERSION 0.0.1)

include("${CMAKE_BINARY_DIR}/conan_paths.cmake")
include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
conan_basic_setup(TARGETS)

add_subdirectory(src)