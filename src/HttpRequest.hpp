#pragma once

#include <nstd/Buffer.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/String.hpp>

class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();

    const String& getErrorString() { return error; }

    bool get(const String& url, Buffer& data, bool checkCertificate = true);

private:
    void* curl;
    String error;
};
