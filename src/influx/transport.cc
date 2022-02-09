#include <iostream>

#include <cstring>

#include <influx/transport.hh>

#include <curl/curl.h>

namespace influx::transport {

namespace {
    struct ReadCallbackData {
        const std::string& body;
        std::size_t cursor = 0;
    };

    struct WriteCallbackData {
        std::string body;
    };

    std::size_t ReadCallback(char *buffer, size_t size, size_t nitems, void *userdata)
    {
        ReadCallbackData& data = *(static_cast<ReadCallbackData*>(userdata));
        std::size_t readSize = std::min(data.body.length() - data.cursor, size * nitems);

        if (readSize == 0) {
            return 0;
        }

        memcpy(buffer, data.body.c_str() + data.cursor, readSize);
        data.cursor += readSize;
        return readSize;
    }

    std::size_t WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        WriteCallbackData& data = *(static_cast<WriteCallbackData*>(userdata));
        std::string str(ptr, size * nmemb);
        data.body += str;
        return str.length();
    }
}

struct HttpSession::Priv {
    CURL* handle;
    std::string host;
    std::string token;
};

HttpSession::HttpSession(const std::string& host, const std::string& token)
    : d_(new Priv)
{
    d_->handle = curl_easy_init();
    d_->host = host;
    d_->token = token;
}

HttpSession::~HttpSession()
{
    curl_easy_cleanup(d_->handle);
}

HttpResponse HttpSession::Get(
    const std::string& url,
    const std::vector<std::pair<std::string, std::string>> headers
)
{
    return Perform(Verb::GET, url, headers, "");
}

HttpResponse HttpSession::Post(
    const std::string& url,
    const std::vector<std::pair<std::string, std::string>> headers,
    const std::string& body
)
{
    return Perform(Verb::POST, url, headers, body);
}

HttpResponse HttpSession::Perform(
    Verb verb,
    const std::string& url,
    const std::vector<std::pair<std::string, std::string>> headers,
    const std::string& body
)
{
    ReadCallbackData source{body};
    WriteCallbackData target;
    curl_easy_setopt(d_->handle, CURLOPT_URL, (d_->host + url).c_str());


    switch (verb) {
        case Verb::GET:
            std::cout << "[GET]";
            break;
        case Verb::POST:
            std::cout << "[POST]";
            curl_easy_setopt(d_->handle, CURLOPT_POST, 1);
            curl_easy_setopt(d_->handle, CURLOPT_POSTFIELDSIZE, source.body.length());
            break;
        default:
            break;
    }

    std::cout << " " << (d_->host + url) << std::endl;

    struct curl_slist* curlHeaders = nullptr;
    for (const auto& header: headers) {
        std::string formatted = header.first + ": " + header.second;
        curlHeaders = curl_slist_append(curlHeaders, formatted.c_str());
    }

    curlHeaders = curl_slist_append(curlHeaders, ("Authorization: Bearer " + d_->token).c_str());

    curl_easy_setopt(d_->handle, CURLOPT_HTTPHEADER, curlHeaders);
    curl_easy_setopt(d_->handle, CURLOPT_READFUNCTION, ReadCallback);
    curl_easy_setopt(d_->handle, CURLOPT_READDATA, (void*)&source);
    curl_easy_setopt(d_->handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(d_->handle, CURLOPT_WRITEDATA, (void*)&target);
    curl_easy_setopt(d_->handle, CURLOPT_FAILONERROR, 1);
    
    auto result = curl_easy_perform(d_->handle);
    if (result != CURLE_OK) {
        int status;
        if (curl_easy_getinfo(d_->handle, CURLINFO_RESPONSE_CODE, &status) == CURLE_OK) {
            throw InfluxRemoteError(status);
        } else {
            throw InfluxError();
        }
    }

    curl_slist_free_all(curlHeaders);

    HttpResponse response;
    curl_easy_getinfo(d_->handle, CURLINFO_RESPONSE_CODE, &response.status);
    response.body = std::move(target.body);

    return response;
}

} // namespace