
#pragma once

#include <nstd/String.h>
#include <nstd/HashSet.h>

#include "Address.h"

struct Settings
{
    Address httpProxyAddr;
    Address listenAddr;
    Address dnsListenAddr;
    bool autoProxySkip;
    HashSet<String> whiteList;
    HashSet<String> blackList;

    Settings();

    static bool isInList(const String& hostname, const HashSet<String>& list);

    static void loadSettings(const String& file, Settings& settings);
};

