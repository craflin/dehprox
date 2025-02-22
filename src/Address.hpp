
#pragma once

#include <nstd/Socket/Socket.hpp>

struct Address
{
    uint32 addr;
    uint16 port;

    Address() : addr(Socket::anyAddress), port(0) {}
};
