
#pragma once

#include <nstd/String.h>

class Hostname
{
public:
    static bool resolve(const String& name, uint32& addr, uint16& port);

    static bool reverseResolveFake(uint32 addr, uint16 port, const String& name);

    static bool reverseResolve(uint32 addr, uint16 port, const String& name);
};
