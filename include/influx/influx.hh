#ifndef INFLUX__INFLUX_HH_
#define INFLUX__INFLUX_HH_

#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include <influx/bucket.hh>
#include <influx/measurement.hh>
#include <influx/flux_parser.hh>

namespace influx {

class Influx {
public:
    Influx(Influx&& other);
    Influx& operator=(Influx&& other);
    Influx(const Influx& other);
    Influx& operator=(const Influx& other);
    Influx(const std::string& host, const std::string& org, const std::string& token);

    ~Influx();

    std::future<Bucket> CreateBucket(const std::string& name, const std::chrono::seconds& dataRetention);
    Bucket GetBucketById(const std::string& id);
    Bucket GetBucketByName(const std::string& name);
    std::vector<Bucket> ListBuckets(std::size_t limit = 20, std::size_t offset = 0);
    void DeleteBucket(Bucket& name);

    std::string QueryRaw(const std::string& flux);
    std::vector<FluxTable> Query(const std::string& flux);

    Bucket operator[](const std::string& name);

private:
    struct Priv;
    std::unique_ptr<Priv> d_;
};

} // namespace

#endif
