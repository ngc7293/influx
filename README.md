# influx-cpp

Modern C++ library for InfluxDB v2

## Example

```cpp
influx::Influx db("http://localhost:8086", org_id, auth_token);
influx::Bucket bucket = db.CreateBucket("MyDataBucket", 24h);

bucket << (influx::Measurement("cpu") << influx::Field("user", 90.0) << influx::Tag("node", node));
bucket.Flush();
```

## Integration

This library is currently designed to be integrated with projects using CMake
and Conan (however, it is not yet available as a Conan package itself). Include
it using `add_subdirectory`. You will need to add the following to your
project's `conanfile.txt`:

- libcurl/7.79.Z
- nlohmann_json/3.10.Z
- gtest/1.11.Z

## Known issues and limitations

- On MSVC `influx::Timestamp` are in hundreds of nanoseconds, not nanoseconds,
  due to MSVC's implementation of `system_clock`.
- Flux query parsing is **very** limited and only supports predictable
  measurement querying. If you do complex query use QueryRaw to get raw output.
  I intend to fix this in the future.
- All communications with the InfluxDB instance are blocking. Threading concerns
  are left to the implementor.
