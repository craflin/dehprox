
#pragma once

#include "Settings.hpp"

#include <nstd/Socket/Socket.hpp>

class DnsServer
{
public:
    DnsServer(const Settings& settings) : _settings(settings), _started(false) {}

    bool start();
    bool isStarted() const {return _started;}

    uint run();

private:
    const Settings& _settings;
    Socket _socket;
    bool _started;
};
