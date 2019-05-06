
#pragma once

#include "Address.h"

#include <nstd/Socket/Socket.h>

class DnsServer
{
public:
    DnsServer(const Address& address) : _address(address) {}

    bool start();

    uint run();

private:
    const Address& _address;
    Socket _socket;
};
