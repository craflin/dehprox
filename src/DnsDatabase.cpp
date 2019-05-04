
#include "DnsDatabase.h"

#include <nstd/Socket/Socket.h>
#include <nstd/Mutex.h>
#include <nstd/HashMap.h>
#include <nstd/Time.h>

#define FAKE_ADDR_SUBNET 0x64400000 // "Shared Address Space for Service Providers"
#define FAKE_ADDR_SUBNET_MASK (~(0xffffffff >> 10))

namespace {

struct AddrInfo
{
    uint32 addr;
    int64 timestamp;
};

Mutex _mutex;
HashMap<String, AddrInfo> _nameToAddr(2000);
HashMap<uint32, String> _addrToName(2000);

void _cleanupAddresses()
{
    int64 now = Time::time();
    while (!_nameToAddr.isEmpty())
    {
        const AddrInfo& addrInfo = *_nameToAddr.begin();
        if (now - addrInfo.timestamp > 15 * 60 * 1000) // 15 minutes
        {
            _addrToName.remove(addrInfo.addr);
            _nameToAddr.removeFront();
        }
        else
            break;
    }
}

}

bool DnsDatabase::resolve(const String& hostname, uint32& addr)
{
   if (!Socket::getHostByName(hostname, addr))
        return false;

    {
        _mutex.lock();
        HashMap<String, AddrInfo>::Iterator it = _nameToAddr.find(hostname);
        if (it != _nameToAddr.end())
        {
            // reset timestamp of stored addr info
            _addrToName.remove(it->addr);
            _nameToAddr.remove(it);
        }
        AddrInfo addrInfo = {addr, Time::time()};
        _nameToAddr.append(hostname, addrInfo);
        _addrToName.append(addr, hostname);
        _mutex.unlock();
    }

    return true;
}

bool DnsDatabase::reverseResolve(uint32 addr, String& hostname)
{
    if ((addr & FAKE_ADDR_SUBNET_MASK) == FAKE_ADDR_SUBNET)
        return false;
    bool result = false;
    {
        _mutex.lock();
        HashMap<uint32, String>::Iterator it = _addrToName.find(addr);
        if (it != _addrToName.end())
        {
            result = true;
            hostname = *it;
        }
        _cleanupAddresses();
        _mutex.unlock();
    }
    return result;
}

uint32 DnsDatabase::resolveFake(const String& hostname)
{
    uint32 fakeAddr;

    {
        _mutex.lock();
        HashMap<String, AddrInfo>::Iterator it = _nameToAddr.find(hostname);
        if (it != _nameToAddr.end())
        {
            fakeAddr = it->addr;

            // reset timestamp of stored addr info
            _nameToAddr.remove(it);
            AddrInfo addrInfo = {fakeAddr, Time::time()};
            _nameToAddr.append(hostname, addrInfo);
        }
        else
        {
            // create a somewhat deterministic unique fake addr using a hashsum of the hostname
            fakeAddr = hash(hostname);
            for (int i = 1;; ++i)
            {
                if (i == 200)
                {
                    fakeAddr = (uint32)Time::microTicks(); // add true randomness after 200 failed unique addr generation attempts
                    i = 0;
                }
                fakeAddr *= 16807;
                fakeAddr ^= ((const char*)hostname)[i % hostname.length()];
                fakeAddr = (fakeAddr & ~FAKE_ADDR_SUBNET_MASK) | FAKE_ADDR_SUBNET;
                if ((fakeAddr & 0xff) == 0xff || (fakeAddr & 0xff00) == 0xff00 || (fakeAddr & 0xff0000) == 0xff0000)
                    continue;
                HashMap<uint32, String>::Iterator it = _addrToName.find(fakeAddr);
                if (it != _addrToName.end())
                    continue;
                break;
            }
            AddrInfo addrInfo = {fakeAddr, Time::time()};
            _nameToAddr.append(hostname, addrInfo);
            _addrToName.append(fakeAddr, hostname);
        }
        _mutex.unlock();
    }

    return fakeAddr;
}

bool DnsDatabase::isFake(uint32 addr)
{
    return (addr & FAKE_ADDR_SUBNET_MASK) == FAKE_ADDR_SUBNET;
}

bool DnsDatabase::reverseResolveFake(uint32 addr, String& hostname)
{
    if ((addr & FAKE_ADDR_SUBNET_MASK) != FAKE_ADDR_SUBNET)
        return false;
    bool result = false;
    {
        _mutex.lock();
        HashMap<uint32, String>::Iterator it = _addrToName.find(addr);
        if (it != _addrToName.end())
        {
            result = true;
            hostname = *it;
        }
        _cleanupAddresses();
        _mutex.unlock();
    }
    return result;
}
