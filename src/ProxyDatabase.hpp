
#pragma once

#include "Address.hpp"

#include <nstd/List.hpp>

class ProxyDatabase
{
public:
    static void add(const List<Address>& proxies, bool permanent);
    static void updateSuccess(const Address& address);
    static void updateFailure(const Address& address);
    static bool getRandom(Address& address);
};
