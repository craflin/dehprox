
#include "ProxyDatabase.hpp"

#include <nstd/Mutex.hpp>
#include <nstd/Array.hpp>
#include <nstd/Math.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/Log.hpp>

namespace {

    struct ProxyData
    {
        Address address;
        bool permanent;
        int rawRating;
        int rating;
    };

    
    Mutex _mutex;
    Array<ProxyData> _proxies;
    HashMap<Address, ProxyData*> _proxiesByAddress;

}

void ProxyDatabase::add(const List<Address>& proxies, bool permanent)
{
    Mutex::Guard guard(_mutex);
    _proxies.reserve(_proxies.size() + proxies.size());
    for (List<Address>::Iterator i = proxies.begin(), end = proxies.end(); i != end; ++i)
    {
        const Address& address = *i;
        if (_proxiesByAddress.find(address) != _proxiesByAddress.end())
            continue;
        ProxyData& proxyData = _proxies.append(ProxyData());
        proxyData.address = address;
        proxyData.permanent = permanent;
        proxyData.rawRating = 0;
        proxyData.rating = 1;
        _proxiesByAddress.append(address, &proxyData);
        Log::debugf("Added proxy address %s:%hu", (const char*)Socket::inetNtoA(address.address), address.port );
    }
}

void ProxyDatabase::updateSuccess(const Address& address)
{
    Mutex::Guard guard(_mutex);
    HashMap<Address, ProxyData*>::Iterator it = _proxiesByAddress.find(address);
    if (it == _proxiesByAddress.end())
        return;
    ProxyData* proxyData = *it;
    if (proxyData->rawRating < 0)
        proxyData->rawRating = 1;
    else if (proxyData->rawRating < 10)
        ++proxyData->rawRating;
    proxyData->rating = proxyData->rawRating;
}

void ProxyDatabase::updateFailure(const Address& address)
{
    Mutex::Guard guard(_mutex);
    HashMap<Address, ProxyData*>::Iterator it = _proxiesByAddress.find(address);
    if (it == _proxiesByAddress.end())
        return;
    ProxyData* proxyData = *it;
    if (proxyData->rawRating > 0)
        proxyData->rawRating = -1;
    else if (proxyData->rawRating > -10)
        --proxyData->rawRating;
    proxyData->rating = 1;
}

bool ProxyDatabase::getRandom(Address& address)
{
    double random = (double)Math::random() / Math::randomMax;

    {
        Mutex::Guard guard(_mutex);
        if (_proxies.isEmpty())
            return false;
        uint totalRating = 0;
        for (Array<ProxyData>::Iterator i = _proxies.begin(), end = _proxies.end(); i != end; ++i)
            totalRating += i->rating;

        int j = (int)(random * totalRating);
        for (Array<ProxyData>::Iterator i = _proxies.begin(), end = _proxies.end(); i != end; ++i)
        {
            const ProxyData& proxyData = *i;
            if (j <= proxyData.rating)
            {
                address = proxyData.address;
                return true;
            }
            j -= proxyData.rating;
        }
        address = _proxies.back().address;
        return true;
    }
}


