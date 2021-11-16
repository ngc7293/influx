add_executable(influx-cpp.test
    main.cpp
    test_measurement.cc
)

target_link_libraries(influx-cpp.test PRIVATE CONAN_PKG::gtest influx-cpp)