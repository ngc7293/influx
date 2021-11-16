#ifndef INFLUX_INFLUX_HH_
#define INFLUX_INFLUX_HH_

#include <chrono>
#include <string>
#include <vector>

namespace influx {

class Bucket;

class InfluxDB {
public:
    InfluxDB(const std::string& host);
    ~InfluxDB();

    Bucket CreateBucket(const std::string& name, std::chrono::seconds& dataRetention);
    Bucket GetBucket(const std::string& name);
    std::vector<Bucket> ListBuckets();
};

}

#endif