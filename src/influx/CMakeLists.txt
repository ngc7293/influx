add_library(influx-cpp STATIC
    bucket.cc
    bucket.hh
    client.cc
    client.hh
    influx.cc
    influx.hh
    measurement.cc
    measurement.hh
    types.hh
)

target_compile_features(influx-cpp PUBLIC cxx_std_20)
target_link_libraries(influx-cpp PRIVATE CONAN_PKG::libcurl CONAN_PKG::nlohmann_json)

add_subdirectory(tests)