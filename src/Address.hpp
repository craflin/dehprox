
#pragma once

#include <nstd/Socket/Socket.hpp>

struct Address
{
    uint32 address;
    uint16 port;

    Address() : address(Socket::anyAddress), port(0) {}
    Address(uint16 port) : address(Socket::anyAddress), port(port) {}
    Address(uint32 addr, uint16 port) : address(addr), port(port) {}
};
