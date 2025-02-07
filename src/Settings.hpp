
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
    bool autoProxySkip;
    HashSet<String> whiteList;
    HashSet<String> blackList;
    HashSet<String> skipProxyList;

    Settings();

    const Address& getProxyAddr(const String& destination) const;

    static bool isInList(const String& hostname, const HashSet<String>& list);

    static void loadSettings(const String& file, Settings& settings);

private:
    typedef HashMap<String, Array<Address>> DestinationHttpProxyAddrsMap;

private:
    Array<Address> _httpProxyAddrs;
    DestinationHttpProxyAddrsMap _destinationHttpProxyAddrs;

private:
    Settings(const Settings&);
    Settings& operator=(const Settings&);
};
