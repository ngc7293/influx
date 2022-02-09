#include <deque>
#include <sstream>

#include <influx/bucket.hh>

namespace influx {

struct Bucket::Priv {
    std::string name;

    size_t batchSize;
    std::deque<Measurement> buffer;
};

Bucket::Bucket(Bucket&& other)
    : d_(new Priv)
{
    d_.swap(other.d_);
}

Bucket::Bucket(const std::string& name)
    : d_(new Priv)
{
    d_->name = name;
}

Bucket::~Bucket()
{
}

void Bucket::Write(const Measurement& measurement)
{
    d_->buffer.push_back(measurement);

    if (d_->buffer.size() >= d_->batchSize) {
        Flush();
    }
}

void Bucket::Write(const std::vector<Measurement>& measurements)
{
    for (const Measurement& measurement: measurements) {
        Write(measurement);
    }
}

std::vector<Measurement> Query(const std::string query)
{
    return {};
}

void Bucket::Flush()
{
    std::stringstream sbuf;
    for (const Measurement& measurement: d_->buffer) {
        sbuf << measurement << std::endl;
    }
}

void Bucket::SetBatchSize(size_t size)
{
    d_->batchSize = size;
}

std::string Bucket::name() const
{
    return d_->name;
}

} // namespace

influx::Bucket& operator<<(influx::Bucket& bucket, const influx::Measurement& measurement)
{
    bucket.Write(measurement);
    return bucket;
}