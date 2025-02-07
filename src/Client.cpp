
#include "Client.hpp"

#ifndef _WIN32
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>
#endif

#include <nstd/Socket/Socket.hpp>
#include <nstd/Log.hpp>
#include <nstd/Time.hpp>

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

Client::Client(Server& server, Server::Client& client, const Address& clientAddr, ICallback& callback, const Settings& settings)
    : _server(server)
    , _handle(client)
    , _callback(callback)
    , _settings(settings)
    , _proxyLine(nullptr)
    , _directLine(nullptr)
    , _activeLine(nullptr)
    , _address(clientAddr)
    , _lastReadActivity(0)
{
    ;
}

Client::~Client()
{
    _server.remove(_handle);
    Log::debugf("%s: Closed client for %s:%hu (%s)", (const char*)Socket::inetNtoA(_address.addr),
            (const char*)Socket::inetNtoA(_destination.addr), _destination.port, (const char*)_destinationHostname);
    delete _proxyLine;
    delete _directLine;
}

bool Client::init()
{
    Socket& clientSocket = _handle.getSocket();
    if (!getOriginalDst(clientSocket, _destination.addr, _destination.port))
        return false;

    bool directConnect = false;
    bool proxyConnect = false;
    const char* rejectReason = nullptr;
    if (DnsDatabase::reverseResolveFake(_destination.addr, _destinationHostname))
        proxyConnect = true;
    else if (DnsDatabase::reverseResolve(_destination.addr, _destinationHostname))
    {
        directConnect = _settings.isAutoProxySkipEnabled();
        proxyConnect = true;
    }
    else if (!DnsDatabase::isFake(_destination.addr))
    {
        _destinationHostname = Socket::inetNtoA(_destination.addr);
        directConnect = _settings.isAutoProxySkipEnabled();
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
        Log::infof("%s: Rejected client for %s:%hu (%s): %s", (const char*)Socket::inetNtoA(_address.addr),
            (const char*)Socket::inetNtoA(_destination.addr), _destination.port, (const char*)_destinationHostname, rejectReason);
        return false;
    }

    if (Settings::isInList(_destinationHostname, _settings.skipProxyList))
    {
        directConnect = true;
        proxyConnect = false;
    }

    Log::debugf("%s: Accepted client for %s:%hu (%s)", (const char*)Socket::inetNtoA(_address.addr),
        (const char*)Socket::inetNtoA(_destination.addr), _destination.port, (const char*)_destinationHostname);

    if (directConnect)
    {
        _directLine = new DirectLine(_server, _handle, *this);
        if (!_directLine->connect(_destination))
            return false;
    }

    if (proxyConnect)
    {
        _proxyLine = new ProxyLine(_server, _handle, *this, _settings);
        if (!_proxyLine->connect(_destinationHostname, _destination.port))
            return false;
    }

    _handle.suspend();
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
    _lastReadActivity = Time::time();
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

void Client::onOpened(DirectLine&)
{
    _activeLine = _directLine->getHandle();
    delete _proxyLine;
    _proxyLine = nullptr;
    Log::infof("%s: Established direct connection with %s:%hu", (const char*)Socket::inetNtoA(_address.addr),
        (const char*)_destinationHostname, _destination.port);
    _handle.resume();
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
    _handle.resume();
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

String Client::getDebugInfo() const
{
    String result("<tr>");

    //<td>192.168.0.196:56269</td><td>77</td><td>active</td><td>8</td><td>k8s.pforgeipt.intra.airbusds.corp:443</td>",
    result += String::fromPrintf("<td>%s:%hu</td><td>%d</td><td>%s</td><td>%d</td><td>%d</td><td>%s:%hu</td>",
        (const char*)Socket::inetNtoA(_address.addr), _address.port, (int)_handle.getSocket().getFileDescriptor(),
        _handle.isSuspended() ? "suspended" : "active", _lastReadActivity ? (int)((Time::time() - _lastReadActivity) / 1000) : (int)-1,
        (int)_handle.getSendBufferSize(),
        (const char*)_destinationHostname, _destination.port);

    if (_proxyLine && !_directLine)
        result.append(_proxyLine->getDebugInfo());
    else if (_directLine && !_proxyLine)
        result.append(_directLine->getDebugInfo());
    else
        result.append(String("<td>connecting</td>"));

    result.append("</tr>");
    return result;
}
