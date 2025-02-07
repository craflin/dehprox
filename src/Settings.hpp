
#pragma once

#include <nstd/Array.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/HashSet.hpp>
#include <nstd/String.hpp>

#include "Address.hpp"

struct Settings
{
    Address listenAddr;
    Address debugListenAddr;
    Address dnsListenAddr;
    HashSet<String> whiteList;
    HashSet<String> blackList;
    HashSet<String> skipProxyList;

public:
    Settings();

    const Address& getProxyAddr(const String& destination) const;
    bool isAutoProxySkipEnabled() const {return _autoProxySkip;}

    static bool isInList(const String& hostname, const HashSet<String>& list);

    void loadSettings(const String& file);

private:
    typedef HashMap<String, Array<Address>> DestinationHttpProxyAddrsMap;

private:
    Array<Address> _httpProxyAddrs;
    DestinationHttpProxyAddrsMap _destinationHttpProxyAddrs;
    bool _autoProxySkip;

private:
    Settings(const Settings&);
    Settings& operator=(const Settings&);
};
