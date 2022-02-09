#ifndef INFLUX_INFLUX_HH_
#define INFLUX_INFLUX_HH_

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace influx {

class Bucket;

class InfluxDB {
public:
    InfluxDB() = delete;
    InfluxDB(const std::string& host, const std::string& org, const std::string& token);
    ~InfluxDB();

    Bucket CreateBucket(const std::string& name, std::chrono::seconds& dataRetention);
    Bucket GetBucket(const std::string& name);
    std::vector<Bucket> ListBuckets();

private:
    struct Priv;
    std::unique_ptr<Priv> d_;
};

}

#endif