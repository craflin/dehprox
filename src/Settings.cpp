
#include "Settings.hpp"

#include <nstd/File.hpp>
#include <nstd/List.hpp>
#include <nstd/Log.hpp>
#include <nstd/Math.hpp>

Settings::Settings(const String& file) : _autoProxySkip(true)
{
    Address defaultHttpProxyAddr;
    defaultHttpProxyAddr.addr = Socket::loopbackAddress;
    defaultHttpProxyAddr.port = 3128;
    _httpProxyAddrs.append(defaultHttpProxyAddr);

    _listenAddr.port = 62124;
    _dnsListenAddr.port = 62124;

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
            _dnsListenAddr.addr = Socket::inetAddr(value, &_dnsListenAddr.port);
        else if (option == "autoProxySkip")
            _autoProxySkip = value.toBool();
        else if (option == "allowDest")
            _whiteList.append(value);
        else if (option == "denyDest")
            _blackList.append(value);
        else if (option == "skipProxyDest")
            _skipProxyList.append(value);
        else if (option == "skipProxyRange")
        {
            List<String> tokens;
            value.split(tokens, "/\\");
            if (tokens.size() < 2)
            {
                Log::warningf("Cannot parse value: %s", (const char*)value);
                continue;
            }
            IpRange range;
            range.network = Socket::inetAddr(*tokens.begin());
            uint32 subnet = (++tokens.begin())->toUInt();
            range.mask = (uint32)-1 << (32 - subnet);
            range.network &= range.mask;
            _skipProxyRanges.append(range);
        }
        else
            Log::warningf("Unknown option: %s", (const char*)option);
    }
}

namespace {
    bool isInList(const String& hostname_, const HashSet<String>& list)
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
}

bool Settings::isInWhiteList(const String& destination) const
{
    return ::isInList(destination, _whiteList);
}

bool Settings::isInBlackList(const String& destination) const
{
    return ::isInList(destination, _blackList);
}

bool Settings::isInSkipProxyList(const String& destination) const
{
    return ::isInList(destination, _skipProxyList);
}

bool Settings::isInSkipProxyRangeList(uint32 ip) const
{
    for (Array<IpRange>::Iterator i = _skipProxyRanges.begin(), end = _skipProxyRanges.end(); i != end; ++i)
    {
        const IpRange& range = *i;
        if ((ip & range.mask) == range.network)
            return true;
    }
    return false;
}

namespace {
    const Address& getRandomProxyAddr(const Array<Address>& addrs)
    {
        return addrs[Math::random() % addrs.size()];
    }
}

const Address& Settings::getProxyAddr(const String& destination_)
{
    DestinationHttpProxyAddrsMap::Iterator it = _destinationHttpProxyAddrs.find(destination_);
    if (it != _destinationHttpProxyAddrs.end())
        return getRandomProxyAddr(*it);
    if (const char* x = destination_.find('.'))
    {
        String destination = destination_.substr(x - (const char*)destination_ + 1);
        for (;;)
        {
            it = _destinationHttpProxyAddrs.find(destination);
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
