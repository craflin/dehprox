
#include "ProxyLine.h"

ProxyLine::ProxyLine(Server& server, Server::Handle& client, ICallback& callback)
    : _server(server)
    , _client(client)
    , _callback(callback)
    , _handle(nullptr)
{
    ;
}

ProxyLine::~ProxyLine()
{
    if (_handle)
        _server.close(*_handle);
}

bool ProxyLine::connect(const String& hostname, int16 port)
{
    _hostname = hostname;
    // todo
    //_handle = _server.connect(addr, port, this);
    //if (!_handle)
    //    return false;
    return true;
}

void ProxyLine::onOpened()
{
    // todo
    //_callback.onOpened(*this);
}

void ProxyLine::onRead()
{
    byte buffer[16384];
    usize size;
    if (!_server.read(*_handle, buffer, sizeof(buffer), size))
        return;
    usize postponed = 0;
    if (!_server.write(_client, buffer, size, &postponed))
        return;
    if (postponed)
        _server.suspend(*_handle);
}

void ProxyLine::onWrite()
{
    _server.resume(_client);
}

void ProxyLine::onClosed()
{
    _callback.onClosed(*this);
}

void ProxyLine::onAbolished()
{
    _callback.onAbolished(*this);
}
