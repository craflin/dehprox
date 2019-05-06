
#pragma once

#include <nstd/String.h>

#include "Address.h"

struct Settings
{
    Address proxyUplink;
    Address proxyListen;
    Address dnsListen;
    bool dnsSurrogate;
    bool autoProxySkip;

    static void loadSettings(const String& file, Settings& settings);
};

