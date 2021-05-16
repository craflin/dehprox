
#include <curl/curl.h>

#include "HttpRequest.hpp"

static class Curl
{
public:
    Curl()
    {
        curl_global_init(CURL_GLOBAL_ALL);
    }
    ~Curl()
    {
        curl_global_cleanup();
    }

} curl;

HttpRequest::HttpRequest()
    : curl(0)
{
}

HttpRequest::~HttpRequest()
{
    if (curl)
        curl_easy_cleanup(curl);
}

bool HttpRequest::get(const String& url, Buffer& data, bool checkCertificate)
{
    if (!curl)
    {
        curl = curl_easy_init();
        if (!curl)
        {
            error = "Could not initialize curl.";
            return false;
        }
    }
    else
        curl_easy_reset(curl);

    struct WriteResult
    {
        static size_t writeResponse(void* ptr, size_t size, size_t nmemb, void* stream)
        {
            WriteResult* result = (struct WriteResult*)stream;
            size_t newBytes = size * nmemb;
            size_t totalSize = result->buffer->size() + newBytes;
            if (totalSize > (size_t)result->buffer->capacity())
                result->buffer->reserve(totalSize * 2);
            result->buffer->append((const byte*)ptr, newBytes);
            return newBytes;
        }

        Buffer* buffer;
    } writeResult = { &data };

    curl_easy_setopt(curl, CURLOPT_URL, (const char*)url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteResult::writeResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeResult);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 40);
    if (!checkCertificate)
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

    data.clear();
    data.reserve(1500);

    CURLcode status = curl_easy_perform(curl);
    if (status != 0)
    {
        error.printf("%s.", curl_easy_strerror(status));
        return false;
    }

    long code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if (code != 200)
    {
        error.printf("Server responded with code %u.", (uint)code);
        return false;
    }

    error.clear();
    return true;
}
