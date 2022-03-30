#include <nlohmann/json.hpp>

#include <influx/client.hh>
#include <influx/influx.hh>

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
    if (dataRetention < 1h && dataRetention != 0s) {
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

    auto response = d_->client.Post("/api/v2/buckets", body.dump());
    auto data = nlohmann::json::parse(response.body);

    return Bucket(
        data["id"],
        data["name"],
        data["orgID"],
        transport::HttpClient(d_->client)
    );
}

Bucket Influx::GetBucketById(const std::string& id)
{
    if (id.empty()) {
        return Bucket();
    }

    auto response = d_->client.Get("/api/v2/buckets/" + id);
    auto data = nlohmann::json::parse(response.body);

    return Bucket(
        data["id"],
        data["name"],
        data["orgID"],
        transport::HttpClient(d_->client)
    );
}

Bucket Influx::GetBucketByName(const std::string& name)
{
    if (name.empty()) {
        return Bucket();
    }

    auto response = d_->client.Get("/api/v2/buckets?name=" + name);
    auto data = nlohmann::json::parse(response.body);

    if (data.count("buckets") && data["buckets"].is_array() && data["buckets"].size() >= 1) {
        const auto& subdata = data["buckets"][0];
        return Bucket(
            subdata["id"],
            subdata["name"],
            subdata["orgID"],
            transport::HttpClient(d_->client)
        );
    }

    return Bucket();
}

std::vector<Bucket> Influx::ListBuckets(std::size_t limit, std::size_t offset)
{
    if (limit < 1 || limit > 100) {
        throw InfluxError("Limit must be within range [1, 100]");
    }

    auto response = d_->client.Get(
        "/api/v2/buckets?limit=" + std::to_string(limit) + "&offset=" + std::to_string(offset)
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
    if (!bucket) {
        return;
    }

    d_->client.Delete("/api/v2/buckets/" + bucket.id());
    bucket = Bucket();
}

std::string Influx::QueryRaw(const std::string& flux)
{
    nlohmann::json body = {
        {"dialect", {
            {"annotations", {"datatype"}},
            {"dateTimeFormat", "RFC3339Nano"},
            {"header", true}
        }},
        {"query", flux}
    };

    auto response = d_->client.Post(
        "/api/v2/query",
        body.dump(),        
        {
            {"Content-Type", "application/json"},
            {"Accept", "application/vnd.influx.arrow"}
        }
    );

    return response.body;
}

std::vector<FluxTable> Influx::Query(const std::string& flux)
{
    auto response = QueryRaw(flux);
    return FluxParser().parse(response);
}

Bucket Influx::operator[](const std::string& name)
{
    return GetBucketByName(name);
}

} // namespace
