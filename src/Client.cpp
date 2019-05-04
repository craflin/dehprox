
#include "Client.h"

#ifndef _WIN32
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>
#endif

#include <nstd/Socket/Socket.h>
#include <nstd/Log.h>

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

Client::Client(Server& server, ICallback& callback)
    : _server(server)
    , _callback(callback)
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

        Log::infof("%s:%hu: Closed client for %s:%hu (%s)", (const char*)Socket::inetNtoA(_address.addr), _address.port, 
            (const char*)Socket::inetNtoA(_destination.addr), _destination.port, (const char*)_destinationHostname);
    }
    delete _proxyLine;
    delete _directLine;
}

bool Client::accept(const Address& proxy, Server::Handle& listener)
{
    _handle = _server.accept(listener, this, &_address.addr, &_address.port, true);
    if (!_handle)
        return false;
    Socket* clientSocket = _server.getSocket(*_handle);
    if (!clientSocket ||
        !getOriginalDst(*clientSocket, _destination.addr, _destination.port))
        return false;

    bool connectDirect = false;
    if (!DnsDatabase::reverseResolveFake(_destination.addr, _destinationHostname))
    {
        connectDirect = true;
        if (!DnsDatabase::reverseResolve(_destination.addr, _destinationHostname))
            _destinationHostname = Socket::inetNtoA(_destination.addr);
    }

    Log::infof("%s:%hu: Accepted client for %s:%hu (%s)", (const char*)Socket::inetNtoA(_address.addr), _address.port, 
        (const char*)Socket::inetNtoA(_destination.addr), _destination.port, (const char*)_destinationHostname);

    if (connectDirect)
    {
        _directLine = new DirectLine(_server, *_handle, *this);
        if (!_directLine->connect(_destination))
            return false;
    }

    _proxyLine = new ProxyLine(_server, *_handle, *this);
    if (!_proxyLine->connect(proxy, _destinationHostname, _destination.port))
        return false;

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
    if (_proxyLine)
    {
        delete _proxyLine;
        _proxyLine = nullptr;
    }
    Log::infof("%s:%hu: Established direct connection with %s:%hu", (const char*)Socket::inetNtoA(_address.addr), _address.port, 
        (const char*)_destinationHostname, _destination.port);
    _server.resume(*_handle);
}

void Client::onClosed(DirectLine&)
{
    delete _directLine;
    _directLine = nullptr;
    if (!_proxyLine)
        _callback.onClosed(*this);
}

void Client::onOpened(ProxyLine&)
{
    _activeLine = _proxyLine->getHandle();
    if (_directLine)
    {
        delete _directLine;
        _directLine = nullptr;
    }
    Log::infof("%s:%hu: Established proxy connection with %s:%hu", (const char*)Socket::inetNtoA(_address.addr), _address.port, 
        (const char*)_destinationHostname, _destination.port);
    _server.resume(*_handle);
}

void Client::onClosed(ProxyLine&)
{
    delete _proxyLine;
    _proxyLine = nullptr;
    if (!_directLine)
        _callback.onClosed(*this);
}
