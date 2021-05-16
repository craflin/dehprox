
#include "ProxyProvider.hpp"
#include "HttpRequest.hpp"
#include "ProxyDatabase.hpp"

#include <nstd/Thread.hpp>
#include <nstd/Log.hpp>

namespace {

    bool matchUInt8(const char*& i, uint8& number)
    {
        if (!String::isDigit(*i))
            return false;
        const char* x = i + 1;
        if (String::isDigit(*x)) ++x;
        if (String::isDigit(*x)) ++x;
        number = (uint8)String::toUInt(i);
        i = x;
        return true;
    }

    bool matchUInt16(const char*& i, uint16& number)
    {
        if (!String::isDigit(*i))
            return false;
        const char* x = i + 1;
        for (int j = 0; j < 4; ++j)
            if (String::isDigit(*x)) ++x;
        number = (uint16)String::toUInt(i);
        i = x;
        return true;
    }

    bool expectChar(const char*& i, char c)
    {
        if (*i != c)
            return false;
        ++i;
        return true;
    }

    bool match(const char*& i, Address& address)
    {
        const char* x = i;
        uint8 a, b, c, d;
        if (!matchUInt8(x, a) ||
            !expectChar(x, '.') ||
            !matchUInt8(x, b) ||
            !expectChar(x, '.') ||
            !matchUInt8(x, c) ||
            !expectChar(x, '.') ||
            !matchUInt8(x, d) ||
            !expectChar(x, ':') ||
            !matchUInt16(x, address.port) ||
            String::isDigit(*x))
            return false;
        i = x;
        address.address = ((uint32)a << 24) | ((uint32)b << 16) | ((uint32)c << 8) | d;
        return true;
    }

    void scan(const char* i)
    {
        Address address;
        List<Address> addresses;
        for (; *i; ++i)
        {
            if (!match(i, address))
                continue;
            addresses.append(address);
        }
        ProxyDatabase::add(addresses, false);
    }

    void update(const String& url)
    {
        HttpRequest request;
        Buffer data;
        if (!request.get(url, data))
        {
            Log::errorf("Could not retrieve '%s': %s", (const char*)url, (const char*)request.getErrorString());
            return;
        }
        scan((const char*)(const byte*)data);
    }
}

uint ProxyProvider::run()
{
    for (;;)
    {
        for (List<String>::Iterator i = _settings.urls.begin(), end = _settings.urls.end(); i != end; ++i)
            update(*i);
        Thread::sleep(_settings.refreshInterval * 1000);
    }
    return 0;
}
