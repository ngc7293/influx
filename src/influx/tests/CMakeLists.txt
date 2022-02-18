add_executable(influx-cpp.test
    config.hh
    main.cpp
    test_bucket.cc
    test_influx.cc
    test_measurement.cc
)

target_link_libraries(influx-cpp.test PRIVATE CONAN_PKG::gtest CONAN_PKG::nlohmann_json influx-cpp)