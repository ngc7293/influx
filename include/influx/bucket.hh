#ifndef INFLUX_BUCKET_HH_
#define INFLUX_BUCKET_HH_

#include <functional>
#include <memory>
#include <vector>

#include <influx/measurement.hh>
#include <influx/client.hh>

namespace influx {

class Bucket {
public:
    Bucket();
    Bucket(Bucket&& other);
    Bucket& operator=(Bucket&& other);
    Bucket(const Bucket& other);
    Bucket& operator=(const Bucket& other);

    ~Bucket();

    operator bool() const;

    void Write(const Measurement& seasurement);
    void Write(const std::vector<Measurement>& seasurements);
    void Flush();

    std::size_t BufferedMeasurementsCount() const;

    std::string id() const;
    std::string name() const;
    std::string orgId() const;
    bool is_system_bucket() const;

private:
    Bucket(const std::string& id, const std::string& name, const std::string& orgId, transport::HttpClient&& client);
    friend class Influx;

private:
    struct Priv;
    std::unique_ptr<Priv> d_;
};

class NullBucketError : public InfluxError {
    const char* what() const throw() override { return "Cannot act on null bucket"; }
};

}

influx::Bucket& operator<<(influx::Bucket& bucket, const influx::Measurement& measurement);

#endif