
#pragma once

#include <nstd/Array.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/HashSet.hpp>
#include <nstd/String.hpp>

#include "Address.hpp"

struct Settings
{
public:
    Settings();

    void loadSettings(const String& file);

    const Address& getListenAddr() const {return _listenAddr;}
    const Address& getDebugListenAddr() const {return _debugListenAddr;}
    const Address& getDnsListenAddr() const {return _dnsListenAddr;}
    const Address& getProxyAddr(const String& destination) const;
    bool isAutoProxySkipEnabled() const {return _autoProxySkip;}
    bool isWhiteListEmpty() const {return _whiteList.isEmpty();}
    bool isInWhiteList(const String& destination) const;
    bool isInBlackList(const String& destination) const;
    bool isInSkipProxyList(const String& destination) const;

private:
    typedef HashMap<String, Array<Address>> DestinationHttpProxyAddrsMap;

private:
    Address _listenAddr;
    Address _debugListenAddr;
    Address _dnsListenAddr;
    Array<Address> _httpProxyAddrs;
    DestinationHttpProxyAddrsMap _destinationHttpProxyAddrs;
    bool _autoProxySkip;
    HashSet<String> _whiteList;
    HashSet<String> _blackList;
    HashSet<String> _skipProxyList;

private:
    Settings(const Settings&);
    Settings& operator=(const Settings&);
};
