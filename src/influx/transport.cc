#include <influx/transport.hh>

#include <curl/curl.h>

namespace influx::transport {

struct HttpSession::Priv {
    std::string domain;
    std::string org;

    CURL* handle;
};

HttpSession::HttpSession(const std::string& domain, const std::string& org)
    : d_(new Priv)
{
    d_->domain = domain;
    d_->org = org;

    d_->handle = curl_easy_init();

}

HttpSession::~HttpSession() = default;

void HttpSession::Post(
    const std::string& path,
    const std::vector<std::pair<std::string, std::string>> headers,
    const std::string& body
)
{
    curl_easy_setopt(d_->handle, CURLOPT_URL, (d_->domain + path).c_str());

}

} // namespace