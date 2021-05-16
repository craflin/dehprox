
#pragma once

#include <nstd/String.hpp>
#include <nstd/HashSet.hpp>
#include <nstd/List.hpp>

#include "Address.hpp"

struct Settings
{
    struct Dns
    {
        Address listenAddress;
        bool resolveAddresses;

        Dns() : listenAddress(62124), resolveAddresses(true) {}
    };

    struct Server
    {
        Address listenAddress;
        List<Address> proxies;
        uint proxyLayers;
        uint connectConcurrency;
        uint connectTimeout;
        uint connectMaxAttempts;
        uint connectionProvision;
      
        Server() : listenAddress(62124),  proxyLayers(0), connectConcurrency(1), connectTimeout(20), connectMaxAttempts(2), connectionProvision(2) {}
    };

    struct Provider
    {
        List<String> urls;
        uint refreshInterval;

        Provider() : refreshInterval(172800) {}
    };

    Dns dns;
    Server server;
    Provider provider;

    HashSet<String> whiteList;
    HashSet<String> blackList;

    static bool isInList(const String& hostname, const HashSet<String>& list);

    static void loadSettings(const String& file, Settings& settings);
};

