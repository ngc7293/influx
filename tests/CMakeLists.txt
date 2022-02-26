add_executable(influx.test
    config.hh
    main.cpp
    test_bucket.cc
    test_flux_parser.cc
    test_influx.cc
    test_measurement.cc
)

target_link_libraries(influx.test PRIVATE CONAN_PKG::gtest CONAN_PKG::nlohmann_json influx)