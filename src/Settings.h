
#pragma once

#include "Address.h"

struct Settings
{
    Address proxyUplink;
    Address proxyListen;
    Address dnsListen;
    bool dnsSurrogate;
    bool autoProxySkip;

    static void loadSettings(Settings& settings);
};

