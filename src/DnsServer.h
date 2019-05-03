
#pragma once

#include "Address.h"

#include <nstd/Socket/Socket.h>

class DnsServer
{
public:
    bool start(const Address& address);

    uint run();

private:
    Socket _socket;
};
