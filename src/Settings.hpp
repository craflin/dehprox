
#pragma once

#include <nstd/String.hpp>
#include <nstd/HashSet.hpp>

#include "Address.hpp"

struct Settings
{
    struct Dns
    {
        Address listenAddress;

        Dns() : listenAddress(62124) {}
    };

    struct Server
    {
        Address listenAddress;
        Address httpProxyAddress;
        bool autoProxySkip;

        Server() : listenAddress(62124), httpProxyAddress(Socket::loopbackAddress, 3128), autoProxySkip(true) {}
    };

    Dns dns;
    Server server;

    HashSet<String> whiteList;
    HashSet<String> blackList;

    static bool isInList(const String& hostname, const HashSet<String>& list);

    static void loadSettings(const String& file, Settings& settings);
};

