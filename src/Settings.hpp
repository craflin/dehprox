
#pragma once

#include <nstd/String.hpp>
#include <nstd/HashSet.hpp>
#include <nstd/Array.hpp>

#include "Address.hpp"

struct Settings
{
    Array<Address> httpProxyAddrs;
    Address listenAddr;
    Address debugListenAddr;
    Address dnsListenAddr;
    bool autoProxySkip;
    HashSet<String> whiteList;
    HashSet<String> blackList;
    HashSet<String> skipProxyList;

    Settings();

    static bool isInList(const String& hostname, const HashSet<String>& list);

    static void loadSettings(const String& file, Settings& settings);
};

