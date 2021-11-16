#ifndef INFLUX_BUCKET_HH_
#define INFLUX_BUCKET_HH_

#include <memory>
#include <vector>

#include <influx/measurement.hh>

namespace influx {

class Bucket {
private: 
    Bucket(const std::string& name);

public:
    ~Bucket();

    void Write(const Measurement& seasurement);
    void Write(const std::vector<Measurement>& seasurements);

    std::vector<Measurement> Query(const std::string query);

    void Flush();
    void SetBatchSize(size_t size);

private:
    struct Priv;
    std::unique_ptr<Priv> d_;
};

}

influx::Bucket& operator<<(influx::Bucket& bucket, const influx::Measurement& measurement);

#endif