
#include "Client.h"

#ifndef _WIN32
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>
#endif

#include <nstd/Socket/Socket.h>

#include "Hostname.h"
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
        _server.close(*_handle);
    delete _proxyLine;
    delete _directLine;
}

bool Client::accept(const Address& proxy, Server::Handle& listener)
{
    Address address;
    _handle = _server.accept(listener, this, &address.addr, &address.port, true);
    if (!_handle)
        return false;
    Socket* clientSocket = _server.getSocket(*_handle);
    if (!clientSocket ||
        !getOriginalDst(*clientSocket, address.addr, address.port))
        return false;

    String hostname;
    if (!Hostname::reverseResolveFake(address.addr, hostname))
    {
        _directLine = new DirectLine(_server, *_handle, *this);
        if (!_directLine->connect(address))
            return false;

        if (!Hostname::reverseResolve(address.addr, hostname))
            hostname = Socket::inetNtoA(address.addr);
    }

    _proxyLine = new ProxyLine(_server, *_handle, *this);
    if (!_proxyLine->connect(proxy, hostname, address.port))
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
}

void Client::onClosed(ProxyLine&)
{
    delete _proxyLine;
    _proxyLine = nullptr;
    if (!_directLine)
        _callback.onClosed(*this);
}
