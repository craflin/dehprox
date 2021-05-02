
#include "DirectLine.hpp"

#include <nstd/Error.hpp>

DirectLine::DirectLine(Server& server, Server::Client& client, ICallback& callback)
    : _server(server)
    , _client(client)
    , _callback(callback)
    , _establisher(nullptr)
    , _handle(nullptr)
{
    ;
}

DirectLine::~DirectLine()
{
    if(_establisher)
        _server.remove(*_establisher);
    if (_handle)
        _server.remove(*_handle);
    
}

bool DirectLine::connect(const Address& address)
{
    _establisher = _server.connect(address.addr, address.port, *this);
    if (!_establisher)
        return false;
    return true;
}

Server::Client::ICallback *DirectLine::onConnected(Server::Client &client)
{
    _handle = &client;
    _callback.onOpened(*this);
    return this;
}

void DirectLine::onAbolished()
{
    _callback.onClosed(*this, Error::getErrorString());
}

void DirectLine::onRead()
{
    byte buffer[262144];
    usize size;
    if (!_handle->read(buffer, sizeof(buffer), size))
        return;
    usize postponed = 0;
    if (!_client.write(buffer, size, &postponed))
        return;
    if (postponed)
        _handle->suspend();
}

void DirectLine::onWrite()
{
    _client.resume();
}

void DirectLine::onClosed()
{
    _callback.onClosed(*this, "Closed by peer");
}
