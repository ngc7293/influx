#ifndef INFLUX_INFLUX_HH_
#define INFLUX_INFLUX_HH_

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace influx {

class Bucket;
class Measurement;

class Influx {
public:
    Influx() = delete;
    Influx(Influx&& other);
    Influx(const Influx& other);
    Influx(const std::string& host, const std::string& org, const std::string& token);
    ~Influx();

    Bucket CreateBucket(const std::string& name, const std::chrono::seconds& dataRetention);
    Bucket GetBucket(const std::string& id);
    std::vector<Bucket> ListBuckets();
    void DeleteBucket(Bucket& name);

    std::vector<Measurement> Query(const std::string& flux);

    Bucket operator[](const std::string& id);

private:
    struct Priv;
    std::unique_ptr<Priv> d_;
};

}

#endif