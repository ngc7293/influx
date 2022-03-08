cmake_minimum_required(VERSION 3.17)
project(influx-cpp VERSION 1.0.0)

if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    include("${CMAKE_BINARY_DIR}/conan_paths.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(TARGETS)
endif()

add_library(influx STATIC
    include/influx/bucket.hh
    include/influx/client.hh
    include/influx/flux_parser.hh
    include/influx/influx.hh
    include/influx/measurement.hh
    include/influx/types.hh
    src/bucket.cc
    src/client.cc
    src/flux_parser.cc
    src/influx.cc
    src/measurement.cc
)

target_include_directories(influx PUBLIC include)
target_compile_features(influx PUBLIC cxx_std_20)
target_link_libraries(influx PRIVATE CONAN_PKG::libcurl CONAN_PKG::nlohmann_json)

if (UNIX)
    target_compile_options(influx PRIVATE -Wall -Werror -Wpedantic -Wno-unknown-pragmas)
elseif(MSVC)
    target_compile_options(influx PRIVATE /W4 /WX /wd4068)
endif()

add_subdirectory(tests)