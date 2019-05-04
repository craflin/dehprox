
#include "Hostname.h"

#include <nstd/Socket/Socket.h>
#include <nstd/Mutex.h>
#include <nstd/HashMap.h>

static Mutex _mutex;
static uint32 _lastFakeAddrBase;
static HashMap<String, uint32> _nameToAddr(2000);
static HashMap<uint32, String> _addrToName(2000);

bool Hostname::resolve(const String& name, uint32& addr)
{
    // todo
    return false;
}

bool Hostname::reverseResolve(uint32 addr, String& name)
{

    // todo
    return false;
}

bool Hostname::reverseResolveFake(uint32 addr, String& name)
{
    bool result = false;
    _mutex.lock();
    HashMap<uint32, String>::Iterator it = _addrToName.find(addr);
    if (it != _addrToName.end())
    {
        result = true;
        name = *it;
    }
    _mutex.unlock();
    return result;
}

uint32 Hostname::resolveFake(const String& hostname)
{
    uint32 fakeAddr;

    _mutex.lock();
    HashMap<String, uint32>::Iterator it = _nameToAddr.find(hostname);
    if (it != _nameToAddr.end())
        fakeAddr = *it;
    else
    {
        do
        {
            fakeAddr = ++_lastFakeAddrBase;
        } while ((fakeAddr & 0xff) == 0xff || (fakeAddr & 0xff00) == 0xff00 || (fakeAddr & 0xff0000) == 0xff0000 || (fakeAddr & 0xff000000) == 0xff000000);
        fakeAddr |= 0x0b000000;
        _nameToAddr.append(hostname, fakeAddr);
        _addrToName.append(fakeAddr, hostname);
    }
    _mutex.unlock();

    return fakeAddr;
}
