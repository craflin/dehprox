
#pragma once

#include <nstd/String.h>

#include "Address.h"

struct Settings
{
    Address httpProxyAddr;
    Address listenAddr;
    Address dnsListenAddr;
    bool autoProxySkip;

    Settings();

    static void loadSettings(const String& file, Settings& settings);
};

