#include <nlohmann/json.hpp>

#include <influx/bucket.hh>
#include <influx/influx.hh>
#include <influx/client.hh>

using namespace std::chrono_literals;

namespace influx {

struct Influx::Priv {
    transport::HttpClient client;
    const std::string org;
};

Influx::Influx(Influx&& other)
    : d_(new Priv)
{
    *this = std::move(other);
}

Influx& Influx::operator=(Influx&& other)
{
    d_.swap(other.d_);
    return *this;
}

Influx::Influx(const Influx& other)
    : d_(new Priv)
{
    *this = other;
}

Influx& Influx::operator=(const Influx& other)
{
    d_.reset(new Priv{other.d_->client, other.d_->org});
    return *this;
}

Influx::Influx(const std::string& host, const std::string& org, const std::string& token)
    : d_(new Priv{{host, org, token}, org})
{
    // TODO: Check provided parameter validity with dummy request
}

Influx::~Influx()
{
}

Bucket Influx::CreateBucket(const std::string& name, const std::chrono::seconds& dataRetention)
{
    if (dataRetention < 1h) {
        throw InfluxError("Retention policy must be at least an hour");
    }

    nlohmann::json body = {
        {"orgID", d_->org},
        {"name", name},
        {"retentionRules", {{
            {"everySeconds", dataRetention.count()},
            {"type", "expire"}
        }}}
    };

    auto response = d_->client.Post(
        "/api/v2/buckets",
        body.dump()
    );

    auto data = nlohmann::json::parse(response.body);
    return Bucket(
        data["id"],
        data["name"],
        data["orgID"],
        transport::HttpClient(d_->client)
    );
}

Bucket Influx::GetBucket(const std::string& id)
{
    auto response = d_->client.Get(
        "/api/v2/buckets/" + id
    );

    auto data = nlohmann::json::parse(response.body);
    return Bucket(
        data["id"],
        data["name"],
        data["orgID"],
        transport::HttpClient(d_->client)
    );
}

std::vector<Bucket> Influx::ListBuckets()
{
    auto response = d_->client.Get(
        "/api/v2/buckets"
    );

    auto data = nlohmann::json::parse(response.body);

    std::vector<Bucket> result;
    for (const auto& bucket: data["buckets"]) {
        result.push_back(Bucket(
            bucket["id"],
            bucket["name"],
            bucket["orgID"],
            transport::HttpClient(d_->client)
        ));
    }

    return result;
}

void Influx::DeleteBucket(Bucket& bucket)
{
    auto response = d_->client.Delete(
        "/api/v2/buckets/" + bucket.id()
    );

    (void) Bucket(std::move(bucket));
}

std::vector<Measurement> Influx::Query(const std::string& flux)
{
    auto response = d_->client.Post(
        "/api/v2/query",
        flux,
        {
            {"Content-Type", "application/vnd.flux"},
            {"Accept", "application/vnd.influx.arrow"}
        }
    );

    std::cout << response.status << " '" << response.body << "'" << std::endl;
    return {};
}

Bucket Influx::operator[](const std::string& id)
{
    return GetBucket(id);
}

}