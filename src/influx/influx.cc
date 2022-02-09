#include <nlohmann/json.hpp>

#include <influx/bucket.hh>
#include <influx/influx.hh>
#include <influx/transport.hh>

namespace influx {

struct InfluxDB::Priv {
    influx::transport::HttpSession session;
    std::string org;
};

InfluxDB::InfluxDB(const std::string& host, const std::string& org, const std::string& token)
    : d_(new Priv{{host, token}, org})
{
}

InfluxDB::~InfluxDB()
{
}

Bucket InfluxDB::CreateBucket(const std::string& name, std::chrono::seconds& dataRetention)
{
    return Bucket("");
}

Bucket InfluxDB::GetBucket(const std::string& name)
{
    return Bucket("");
}

std::vector<Bucket> InfluxDB::ListBuckets()
{
    auto response = d_->session.Get(
        "/api/v2/buckets?orgId=" + d_->org,
        {}
    );

    auto data = nlohmann::json::parse(response.body);

    std::vector<Bucket> result;
    for (const auto& bucket: data["buckets"]) {
        result.push_back(Bucket(bucket["name"].get<std::string>()));
    }

    return result;
}

}