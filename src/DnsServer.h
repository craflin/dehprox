
#pragma once

#include "Settings.h"

#include <nstd/Socket/Socket.h>

class DnsServer
{
public:
    DnsServer(const Settings& settings) : _settings(settings) {}

    bool start();

    uint run();

private:
    const Settings& _settings;
    Socket _socket;
};
