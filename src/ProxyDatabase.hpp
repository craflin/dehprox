
#pragma once

#include "Address.hpp"

#include <nstd/HashSet.hpp>

class ProxyDatabase
{
public:
    static void add(const HashSet<Address>& proxies, bool permanent);
    static void updateSuccess(const Address& address);
    static void updateFailure(const Address& address);
    static bool getRandom(Address& address);
};
