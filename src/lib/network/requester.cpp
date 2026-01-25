#include <string>
#include <sstream>
#include <curl/curl.h>
#include "lib/network/requester.hpp"

static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t real_size = size * nmemb;

    std::stringstream *ss = static_cast<std::stringstream *>(userp);
    ss->write(static_cast<char *>(contents), real_size);

    return real_size;
}

static std::string performBodyRequest(const std::string &url, const std::vector<std::string> &headers, const std::string &body, const char *method)
{
    CURL *curlHandle = curl_easy_init();

    if (!curlHandle)
    {
        throw std::runtime_error("could not initialize curl handle");
    }

    struct curl_slist *curlHeaders = nullptr;
    for (const auto &header : headers)
    {
        curlHeaders = curl_slist_append(curlHeaders, header.c_str());
    }

    std::stringstream response;
    curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, curlHeaders);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curlHandle, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &response);

    if (method != nullptr)
    {
        curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, method);
    }

    CURLcode res = curl_easy_perform(curlHandle);

    int responseHttpCode;
    curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &responseHttpCode);

    curl_slist_free_all(curlHeaders);
    curl_easy_cleanup(curlHandle);

    if (res != CURLE_OK)
    {
        std::string errorMessage = "curl failed: ";
        errorMessage.append(curl_easy_strerror(res));
        throw std::runtime_error(errorMessage);
    }

    if (responseHttpCode != 200)
    {
        std::string errorMessage = "request failed: " + std::to_string(responseHttpCode);
        throw std::runtime_error(errorMessage);
    }

    return response.str();
}

namespace network
{
    Requester::Requester()
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    Requester::~Requester()
    {
    }

    std::string Requester::getRequest(const std::string &url, const std::vector<std::string> &headers)
    {
        CURL *curlHandle = curl_easy_init();

        if (!curlHandle)
        {
            throw std::runtime_error("could not initialize curl handle");
        }

        struct curl_slist *curlHeaders = nullptr;
        for (auto &header : headers)
        {
            curlHeaders = curl_slist_append(curlHeaders, header.c_str());
        }

        curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, curlHeaders);
        curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curlHandle, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_1_1);
        std::stringstream response;
        curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curlHandle);

        curl_slist_free_all(curlHeaders);
        curl_easy_cleanup(curlHandle);

        if (res != CURLE_OK)
        {
            std::string errorMessage = "curl failed: ";
            errorMessage.append(curl_easy_strerror(res));
            throw std::runtime_error(errorMessage);
        }

        return response.str();
    }

    std::string Requester::postRequest(const std::string &url, const std::vector<std::string> &headers, const std::string &body)
    {
        return performBodyRequest(url, headers, body, "POST");
    }

    std::string Requester::putRequest(const std::string &url, const std::vector<std::string> &headers, const std::string &body)
    {
        return performBodyRequest(url, headers, body, "PUT");
    }
}
