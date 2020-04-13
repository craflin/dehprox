
#pragma once

#include "Settings.hpp"

#include <nstd/Socket/Socket.hpp>

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
