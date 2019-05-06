
#pragma once

#include <nstd/Base.h>
#include <nstd/Socket/Socket.h>

struct Address
{
    uint32 addr;
    uint16 port;

    Address() : addr(Socket::anyAddr), port(0) {}
};
