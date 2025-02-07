
#include "Settings.hpp"

#include <nstd/File.hpp>
#include <nstd/List.hpp>
#include <nstd/Log.hpp>
#include <nstd/Math.hpp>

Settings::Settings() : _autoProxySkip(true)
{
    Address defaultHttpProxyAddr;
    defaultHttpProxyAddr.addr = Socket::loopbackAddress;
    defaultHttpProxyAddr.port = 3128;
    _httpProxyAddrs.append(defaultHttpProxyAddr);

    _listenAddr.port = 62124;
    dnsListenAddr.port = 62124;
}

void Settings::loadSettings(const String& file)
{
    String conf;
    if (!File::readAll(file, conf))
        return;
    List<String> lines;
    conf.split(lines, "\n\r");
    bool httpProxyAddrsSet = false;
    for (List<String>::Iterator i = lines.begin(), end = lines.end(); i != end; ++i)
    {
        String line = *i;
        const char* lineEnd = line.find('#');
        if (lineEnd)
            line.resize(lineEnd - (const char*)line);
        line.trim();
        if (line.isEmpty())
            continue;
        List<String> tokens;
        line.split(tokens, " \t");
        if (tokens.size() < 2)
            continue;
        const String& option = *tokens.begin();
        const String& value = *(++tokens.begin());
        if (option == "httpProxyAddr")
        {
            if (!httpProxyAddrsSet)
            {
                httpProxyAddrsSet = true;
                _httpProxyAddrs.clear();
            }

            Address addr;
            addr.addr = Socket::inetAddr(value, &addr.port);

            if (tokens.size() > 2)
            {
                const String& destination = *(++(++tokens.begin()));

                DestinationHttpProxyAddrsMap::Iterator it = _destinationHttpProxyAddrs.find(destination);
                if (it == _destinationHttpProxyAddrs.end())
                    it = _destinationHttpProxyAddrs.insert(_destinationHttpProxyAddrs.end(), destination, Array<Address>());
                it->append(addr);
            }
            else
                _httpProxyAddrs.append(addr);
        }
        else if (option == "listenAddr")
            _listenAddr.addr = Socket::inetAddr(value, &_listenAddr.port);
        else if (option == "debugListenAddr")
            _debugListenAddr.addr = Socket::inetAddr(value, &_debugListenAddr.port);
        else if (option == "dnsListenAddr")
            dnsListenAddr.addr = Socket::inetAddr(value, &dnsListenAddr.port);
        else if (option == "autoProxySkip")
            _autoProxySkip = value.toBool();
        else if (option == "allowDest")
            whiteList.append(value);
        else if (option == "denyDest")
            blackList.append(value);
        else if (option == "skipProxyDest")
            skipProxyList.append(value);
        else
            Log::warningf("Unknown option: %s", (const char*)option);
    }
}

bool Settings::isInList(const String& hostname_, const HashSet<String>& list)
{
    if (list.contains(hostname_))
        return true;
    const char* x = hostname_.find('.');
    if (!x)
        return false;
    String hostname = hostname_.substr(x - (const char*)hostname_ + 1);
    for (;;)
    {
        if (list.contains(hostname))
            return true;
        const char* x = hostname.find('.');
        if (!x)
            return false;
        hostname = hostname.substr(x - (const char*)hostname + 1);
    }
}

namespace {
    const Address& getRandomProxyAddr(const Array<Address>& addrs)
    {
        return addrs[Math::random() % addrs.size()];
    }
}

const Address& Settings::getProxyAddr(const String& destination_) const
{
    DestinationHttpProxyAddrsMap::Iterator it = _destinationHttpProxyAddrs.find(destination_);
    if (it != _destinationHttpProxyAddrs.end())
        return getRandomProxyAddr(*it);
    if (const char* x = destination_.find('.'))
    {
        String destination = destination_.substr(x - (const char*)destination_ + 1);
        for (;;)
        {
            it = _destinationHttpProxyAddrs.find(destination_);
            if (it != _destinationHttpProxyAddrs.end())
                return getRandomProxyAddr(*it);
            const char* x = destination.find('.');
            if (!x)
                break;
            destination = destination.substr(x - (const char*)destination + 1);
        }
    }
    return getRandomProxyAddr(_httpProxyAddrs);
}
