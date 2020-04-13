
#include "Client.h"

#ifndef _WIN32
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>
#endif

#include <nstd/Socket/Socket.hpp>
#include <nstd/Log.hpp>

#include "DnsDatabase.h"
#include "DirectLine.h"
#include "ProxyLine.h"

bool getOriginalDst(Socket& s, uint32& addr, uint16& port)
{
#ifdef _WIN32
    return s.getSockName(addr, port);
#else
    sockaddr_in destAddr;
    usize destAddrLen = sizeof(destAddr);
    if(!s.getSockOpt(SOL_IP, SO_ORIGINAL_DST, &destAddr, destAddrLen))
        return false;
    addr = ntohl(destAddr.sin_addr.s_addr);
    port = ntohs(destAddr.sin_port);
    return true;
#endif
}

Client::Client(Server& server, ICallback& callback, const Settings& settings)
    : _server(server)
    , _callback(callback)
    , _settings(settings)
    , _handle(nullptr)
    , _proxyLine(nullptr)
    , _directLine(nullptr)
    , _activeLine(nullptr)
{
    ;
}

Client::~Client()
{
    if (_handle)
    {
        _server.close(*_handle);

        Log::debugf("%s: Closed client for %s:%hu (%s)", (const char*)Socket::inetNtoA(_address.addr),
            (const char*)Socket::inetNtoA(_destination.addr), _destination.port, (const char*)_destinationHostname);
    }
    delete _proxyLine;
    delete _directLine;
}

bool Client::accept(Server::Handle& listener)
{
    _handle = _server.accept(listener, this, &_address.addr, &_address.port, true);
    if (!_handle)
        return false;
    Socket* clientSocket = _server.getSocket(*_handle);
    if (!clientSocket ||
        !getOriginalDst(*clientSocket, _destination.addr, _destination.port))
        return false;

    bool directConnect = false;
    bool proxyConnect = false;
    const char* rejectReason = nullptr;
    if (DnsDatabase::reverseResolveFake(_destination.addr, _destinationHostname))
        proxyConnect = true;
    else if (DnsDatabase::reverseResolve(_destination.addr, _destinationHostname))
    {
        directConnect = _settings.autoProxySkip;
        proxyConnect = true;
    }
    else if (!DnsDatabase::isFake(_destination.addr))
    {
        _destinationHostname = Socket::inetNtoA(_destination.addr);
        directConnect = _settings.autoProxySkip;
        proxyConnect = true;
    }
    else
        rejectReason = "Unknown surrogate address";

    if (!rejectReason)
    {
        if (!_settings.whiteList.isEmpty() && !Settings::isInList(_destinationHostname, _settings.whiteList))
            rejectReason = "Not listed in white list";
        else if (Settings::isInList(_destinationHostname, _settings.blackList))
            rejectReason = "Listed in black list";
    }

    if (rejectReason)
    {
        _server.close(*_handle);
        _handle = nullptr;
        Log::infof("%s: Rejected client for %s:%hu (%s): %s", (const char*)Socket::inetNtoA(_address.addr),
            (const char*)Socket::inetNtoA(_destination.addr), _destination.port, (const char*)_destinationHostname, rejectReason);
        return false;
    }

    Log::debugf("%s: Accepted client for %s:%hu (%s)", (const char*)Socket::inetNtoA(_address.addr),
        (const char*)Socket::inetNtoA(_destination.addr), _destination.port, (const char*)_destinationHostname);

    if (directConnect)
    {
        _directLine = new DirectLine(_server, *_handle, *this);
        if (!_directLine->connect(_destination))
            return false;
    }

    if (proxyConnect)
    {
        _proxyLine = new ProxyLine(_server, *_handle, *this, _settings);
        if (!_proxyLine->connect(_destinationHostname, _destination.port))
            return false;
    }

    return true;
}

void Client::onRead()
{
    byte buffer[262144];
    usize size;
    if (!_server.read(*_handle, buffer, sizeof(buffer), size))
        return;
    usize postponed = 0;
    if (!_server.write(*_activeLine, buffer, size, &postponed))
        return;
    if (postponed)
        _server.suspend(*_handle);
}

void Client::onWrite()
{
    if (_activeLine)
        _server.resume(*_activeLine);
}

void Client::onClosed()
{
    _callback.onClosed(*this);
}

void Client::onOpened(DirectLine&)
{
    _activeLine = _directLine->getHandle();
    delete _proxyLine;
    _proxyLine = nullptr;
    Log::infof("%s: Established direct connection with %s:%hu", (const char*)Socket::inetNtoA(_address.addr),
        (const char*)_destinationHostname, _destination.port);
    _server.resume(*_handle);
}

void Client::onClosed(DirectLine&, const String& error)
{
    delete _directLine;
    _directLine = nullptr;
    if (!_proxyLine)
        close(error);
}

void Client::onOpened(ProxyLine&)
{
    _activeLine = _proxyLine->getHandle();
    delete _directLine;
    _directLine = nullptr;
    Log::infof("%s: Established proxy connection with %s:%hu", (const char*)Socket::inetNtoA(_address.addr),
        (const char*)_destinationHostname, _destination.port);
    _server.resume(*_handle);
}

void Client::onClosed(ProxyLine&, const String& error)
{
    delete _proxyLine;
    _proxyLine = nullptr;
    if (!_directLine)
        close(error);
}

void Client::close(const String& error)
{
    if (!_activeLine)
        Log::infof("%s: Failed to establish connection with %s:%hu: %s", (const char*)Socket::inetNtoA(_address.addr),
            (const char*)_destinationHostname, _destination.port, (const char*)error);
    _callback.onClosed(*this);
}
