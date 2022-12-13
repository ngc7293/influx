#include <algorithm>

#include <cassert>
#include <cstring>

#define NOMINMAX
#include <curl/curl.h>

#ifdef DELETE
#undef DELETE // Damn you Windows
#endif

#include <influx/client.hh>

#include "util.hh"

namespace influx::transport {

namespace {
    struct ReadCallbackData {
        const std::string& body;
        std::size_t cursor = 0;
    };

    struct WriteCallbackData {
        std::string body;
    };

    std::size_t ReadCallback(char *buffer, std::size_t size, std::size_t nitems, void *userdata)
    {
        ReadCallbackData& data = *(static_cast<ReadCallbackData*>(userdata));
        std::size_t readSize = std::min(data.body.length() - data.cursor, size * nitems);

        if (readSize == 0) {
            return 0;
        }

        memcpy(static_cast<void*>(buffer), data.body.c_str() + data.cursor, readSize);
        data.cursor += readSize;
        return readSize;
    }

    std::size_t WriteCallback(const char *ptr, std::size_t size, std::size_t nmemb, void *userdata)
    {
        WriteCallbackData& data = *(static_cast<WriteCallbackData*>(userdata));
        std::string str(ptr, size * nmemb);
        data.body += str;
        return str.length();
    }
}

struct HttpClient::Priv {
    const std::string host, org, token;
   
    std::string makeUrl(const std::string& endpoint)
    {
        std::string out;

        if (endpoint.find("?") == std::string::npos) {
            out = "?orgID=";
        } else {
            out = "&orgID=";
        }

        return host + endpoint + out + org;
    }

    struct curl_slist* makeHeaders(const std::unordered_map<std::string, std::string>& headers)
    {
        struct curl_slist* curlHeaders = curl_slist_append(nullptr, ("Authorization: Bearer " + token).c_str());

        std::for_each(headers.begin(), headers.end(), [&](const auto& kvp) {
            std::string formatted = kvp.first + ": " + kvp.second;
            curlHeaders = curl_slist_append(curlHeaders, formatted.c_str());
        });

        return curlHeaders;
    }
};

HttpClient::HttpClient()
    : d_(new Priv)
{
}

HttpClient::HttpClient(HttpClient&& other)
    : d_(new Priv)
{
    d_.swap(other.d_);
}

HttpClient::HttpClient(const HttpClient& other)
    : d_(new Priv{other.d_->host, other.d_->org, other.d_->token})
{
}

HttpClient::HttpClient(const std::string& host, const std::string& org, const std::string& token)
    : d_(new Priv{host, org, token})
{
}

HttpClient::~HttpClient()
{
}

HttpResponse HttpClient::Get(
    const std::string& endpoint,
    const std::unordered_map<std::string, std::string>& headers
)
{
    return Perform(Verb::GET, endpoint, std::string(), headers);
}

HttpResponse HttpClient::Post(
    const std::string& endpoint,
    const std::string& body,
    const std::unordered_map<std::string, std::string>& headers
)
{
    return Perform(Verb::POST, endpoint, body, headers);
}

HttpResponse HttpClient::Delete(
    const std::string& endpoint,
    const std::string& body,
    const std::unordered_map<std::string, std::string>& headers
)
{
    return Perform(Verb::DELETE, endpoint, body, headers);
}

HttpResponse HttpClient::Perform(
    Verb verb,
    const std::string& endpoint,
    const std::string& body,
    const std::unordered_map<std::string, std::string>& headers
)
{
    ReadCallbackData source{body};
    WriteCallbackData target;

    CURL* handle = curl_easy_init();
    curl_easy_reset(handle);
    curl_easy_setopt(handle, CURLOPT_URL, d_->makeUrl(endpoint).c_str());

    switch (verb) {
        case Verb::GET:
            break;
        case Verb::POST:
            curl_easy_setopt(handle, CURLOPT_POST, 1);
            curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, source.body.length());
            break;
        case Verb::DELETE:
            curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, source.body.length());
            break;
        default:
            assert(false);
            break;
    }

    struct curl_slist* curlHeaders = d_->makeHeaders(headers);
    auto _ = finally([&]() { curl_slist_free_all(curlHeaders); });

    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, curlHeaders);
    curl_easy_setopt(handle, CURLOPT_READFUNCTION, ReadCallback);
    curl_easy_setopt(handle, CURLOPT_READDATA, (void*)&source);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&target);
    
    if (curl_easy_perform(handle) != CURLE_OK) {
        curl_easy_cleanup(handle);
        throw InfluxError();
    }

    int status;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status);

    if (status / 100 > 2) {
        curl_easy_cleanup(handle);
        throw InfluxRemoteError(status, std::move(target.body));
    } else {
        curl_easy_cleanup(handle);
        return {status, std::move(target.body)};
    }
}

} // namespace
