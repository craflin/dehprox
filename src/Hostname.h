
#pragma once

#include <nstd/String.h>

class Hostname
{
public:
    static bool resolve(const String& name, uint32& addr);

    static bool reverseResolveFake(uint32 addr, const String& name);

    static bool reverseResolve(uint32 addr, const String& name);
};
