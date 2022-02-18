#include <deque>
#include <iostream>  // FIXME: remove
#include <sstream>

#include <cassert>

#include <influx/bucket.hh>
#include <influx/client.hh>

namespace influx {

struct Bucket::Priv {
    // From API
    std::string id;
    std::string name;
    std::string orgId;

    // Local data
    transport::HttpClient client;  
    std::deque<Measurement> buffer;
};

Bucket::Bucket()
    : d_(new Priv)
{
}

Bucket::Bucket(Bucket&& other)
    : d_(new Priv)
{
    d_.swap(other.d_);
}

Bucket::Bucket(const Bucket& other)
    : d_(new Priv{other.d_->id, other.d_->name, other.d_->orgId, other.d_->client})
{
}

Bucket::Bucket(const std::string& id, const std::string& name, const std::string& orgId, transport::HttpClient&& client)
    : d_(new Priv{id, name, orgId, std::move(client)})
{
}

Bucket::~Bucket()
{
}

void Bucket::Write(const Measurement& measurement)
{
    if (d_->id.empty()) {
        throw NullBucketError();
    }

    d_->buffer.push_back(measurement);
}

void Bucket::Write(const std::vector<Measurement>& measurements)
{
    if (d_->id.empty()) {
        throw NullBucketError();
    }

    for (const Measurement& measurement: measurements) {
        Write(measurement);
    }
}

void Bucket::Flush()
{
    if (d_->id.empty()) {
        throw NullBucketError();
    }

    std::stringstream sbuf;
    for (const Measurement& measurement: d_->buffer) {
        sbuf << measurement << std::endl;
    }

    d_->client.Post(
        "/api/v2/write?bucket=" + d_->id,
        sbuf.str()
    );

    d_->buffer.clear();
}

std::size_t Bucket::BufferedMeasurementsCount() const
{
    return d_->buffer.size();
}

std::string Bucket::id() const
{
    return d_->id;
}

std::string Bucket::name() const
{
    return d_->name;
}

std::string Bucket::orgId() const
{
    return d_->orgId;
}

bool Bucket::is_system_bucket() const
{
    return d_->name.starts_with("_");
}

} // namespace

influx::Bucket& operator<<(influx::Bucket& bucket, const influx::Measurement& measurement)
{
    bucket.Write(measurement);
    return bucket;
}