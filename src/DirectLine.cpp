
#include "DirectLine.h"

DirectLine::DirectLine(Server& server, Server::Handle& client, ICallback& callback)
    : _server(server)
    , _client(client)
    , _callback(callback)
    , _handle(nullptr)
{
    ;
}

DirectLine::~DirectLine()
{
    if (_handle)
        _server.close(*_handle);
}

bool DirectLine::connect(const Address& address)
{
    _handle = _server.connect(address.addr, address.port, this);
    if (!_handle)
        return false;
    return true;
}

void DirectLine::onOpened()
{
    _callback.onOpened(*this);
}

void DirectLine::onRead()
{
    byte buffer[262144];
    usize size;
    if (!_server.read(*_handle, buffer, sizeof(buffer), size))
        return;
    usize postponed = 0;
    if (!_server.write(_client, buffer, size, &postponed))
        return;
    if (postponed)
        _server.suspend(*_handle);
}

void DirectLine::onWrite()
{
    _server.resume(_client);
}

void DirectLine::onClosed()
{
    _callback.onClosed(*this);
}

void DirectLine::onAbolished()
{
    _callback.onClosed(*this);
}
