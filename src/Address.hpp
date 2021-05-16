
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

inline String toString(const Address& address)
{
    return Socket::inetNtoA(address.address) + String::fromUInt((uint)address.port);
}

inline usize hash(const Address& address)
{
    return (address.port << 16) ^ address.address;
}

inline bool operator==(const Address& lh, const Address& rh)
{
    return lh.address == rh.address && lh.port == rh.port;
}
