
#include "Client.hpp"

#ifndef _WIN32
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>
#endif

#include <nstd/Socket/Socket.hpp>
#include <nstd/Log.hpp>

#include "DnsDatabase.hpp"
#include "DirectLine.hpp"
#include "ProxyLine.hpp"

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

Client::Client(Server& server, Server::Client& client, ICallback& callback, const Settings& settings)
    : _server(server)
    , _handle(client)
    , _callback(callback)
    , _settings(settings)
    , _directLine(nullptr)
    , _activeLine(nullptr)
    , _failedProxyConnections(0)
{
    ;
}

Client::~Client()
{
    _server.remove(_handle);
    Log::debugf("%s: Closed client for %s:%hu (%s)", (const char*)Socket::inetNtoA(_address.address),
            (const char*)Socket::inetNtoA(_destination.address), _destination.port, (const char*)_destinationHostname);
    delete _directLine;
}

bool Client::init()
{
    Socket& clientSocket = _handle.getSocket();
    if (!getOriginalDst(clientSocket, _destination.address, _destination.port))
        return false;

    bool directConnect = false;
    const char* rejectReason = nullptr;
    if (DnsDatabase::reverseResolveFake(_destination.address, _destinationHostname))
    {
        ;
    }
    else if (DnsDatabase::reverseResolve(_destination.address, _destinationHostname))
    {
        directConnect = _settings.server.proxyLayers == 0;
    }
    else if (!DnsDatabase::isFake(_destination.address))
    {
        _destinationHostname = Socket::inetNtoA(_destination.address);
        directConnect = _settings.server.proxyLayers == 0;
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
        Log::infof("%s: Rejected client for %s:%hu (%s): %s", (const char*)Socket::inetNtoA(_address.address),
            (const char*)Socket::inetNtoA(_destination.address), _destination.port, (const char*)_destinationHostname, rejectReason);
        return false;
    }

    Log::debugf("%s: Accepted client for %s:%hu (%s)", (const char*)Socket::inetNtoA(_address.address),
        (const char*)Socket::inetNtoA(_destination.address), _destination.port, (const char*)_destinationHostname);

    if (directConnect)
    {
        _directLine = new DirectLine(_server, _handle, *this);
        if (!_directLine->connect(_destination))
            return false;
    }

    _handle.suspend();
    return true;
}

bool Client::connect(ProxyConnection& proxy)
{
    ProxyLine& proxyLine = _proxyLines.append<Server&, Server::Client&,  ProxyConnection&, ProxyLine::ICallback&, const Settings&>(_server, _handle, proxy, *this, _settings);
    if (!proxyLine.connect(_destinationHostname, _destination.port))
    {
        _proxyLines.remove(proxyLine);
        return false;
    }
    return true;
}

void Client::onRead()
{
    byte buffer[262144];
    usize size;
    if (!_handle.read(buffer, sizeof(buffer), size))
        return;
    usize postponed = 0;
    if (!_activeLine->write(buffer, size, &postponed))
        return;
    if (postponed)
        _handle.suspend();
}

void Client::onWrite()
{
    if (_activeLine)
        _activeLine->resume();
}

void Client::onClosed()
{
    _callback.onClosed(*this);
}

void Client::onConnected(DirectLine&)
{
    _activeLine = _directLine->getHandle();

    Log::infof("%s: Established direct connection with %s:%hu", (const char*)Socket::inetNtoA(_address.address),
        (const char*)_destinationHostname, _destination.port);
    _handle.resume();

    _callback.onEstablished(*this);

    for (PoolList<ProxyLine>::Iterator i = _proxyLines.begin(), end = _proxyLines.begin(); i != end;)
    {
        ProxyLine& proxy = *i;
        i = _proxyLines.remove(i);
        _callback.onClosed(proxy.getProxyConnection());
    }
    
}

void Client::onClosed(DirectLine&, const String& error)
{
    delete _directLine;
    _directLine = nullptr;

    if (_failedProxyConnections >= _settings.server.connectMaxAttempts)
        close(error);
}

void Client::onConnected(ProxyLine& proxyLine)
{
    _activeLine = &proxyLine.getHandle();
    delete _directLine;
    _directLine = nullptr;

    Log::infof("%s: Established proxy connection with %s:%hu", (const char*)Socket::inetNtoA(_address.address),
        (const char*)_destinationHostname, _destination.port);
    _handle.resume();

    _callback.onEstablished(*this);

    for (PoolList<ProxyLine>::Iterator i = _proxyLines.begin(), end = _proxyLines.begin(); i != end;)
    {
        ProxyLine& otherLine = *i;
        if (&otherLine != &proxyLine)
        {
            i = _proxyLines.remove(i);
            _callback.onClosed(otherLine.getProxyConnection());
        }
        else
            ++i;
    }
}

void Client::onClosed(ProxyLine& proxyLine, const String& error)
{
    _proxyLines.remove(proxyLine);
    _callback.onClosed(proxyLine.getProxyConnection());

    ++_failedProxyConnections;
    if (_failedProxyConnections >= _settings.server.connectMaxAttempts && !_directLine)
        close(error);
    else
        _callback.onProxyFailed(*this);
}

void Client::close(const String& error)
{
    if (!_activeLine)
        Log::infof("%s: Failed to establish connection with %s:%hu: %s", (const char*)Socket::inetNtoA(_address.address),
        (const char*)_destinationHostname, _destination.port, (const char*)error);
    _callback.onClosed(*this);
}
