
#pragma once

#include <nstd/String.h>

class Hostname
{
public:
    static bool resolve(const String& hostname, uint32& addr);

    static bool reverseResolve(uint32 addr, String& hostname);

    static uint32 resolveFake(const String& hostname);

    static bool reverseResolveFake(uint32 addr, String& hostname);
};
