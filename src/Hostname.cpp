
#include "Hostname.h"

#include <nstd/Socket/Socket.h>
#include <nstd/Mutex.h>

static Mutex _mutex;
static uint32 _lastFakeIp;

bool Hostname::resolve(const String& name, uint32& addr)
{
    // todo
    return false;
}

bool Hostname::reverseResolveFake(uint32 addr, const String& name)
{
    // todo
    return false;
}

uint32 Hostname::resolveFake(const String& hostname)
{
    uint32 fakeIp;
    do {
        fakeIp = ++_lastFakeIp;
    } while ((fakeIp & 0xff) == 0xff || (fakeIp & 0xff00) == 0xff00 || (fakeIp & 0xff0000) == 0xff0000 || (fakeIp & 0xff000000) == 0xff000000);

    // todo
    return Socket::inetAddr("10.6.23.1");
}

bool Hostname::reverseResolve(uint32 addr, const String& name)
{
    // todo
    return false;
}
