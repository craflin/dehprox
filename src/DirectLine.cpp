
#include "DirectLine.hpp"

#include <nstd/Error.hpp>
#include <nstd/Time.hpp>

DirectLine::DirectLine(Server& server, Server::Client& client, ICallback& callback)
    : _server(server)
    , _client(client)
    , _callback(callback)
    , _establisher(nullptr)
    , _handle(nullptr)
    , _lastReadActivity(0)
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
    _lastReadActivity = Time::time();
}

void DirectLine::onWrite()
{
    _client.resume();
}

void DirectLine::onClosed()
{
    _callback.onClosed(*this, "Closed by peer");
}

String DirectLine::getDebugInfo() const
{
    if (!_handle)
        return String("<td>connecting</td>");

    uint32 ip = 0;
    uint16 port = 0;
    _handle->getSocket().getSockName(ip, port);

    //<td>direct</td><td>0.0.0.0:828</td><td>78</td><td>active</td><td>8</td>
    return String::fromPrintf("<td>direct</td><td>%s:%hu</td><td>%d</td><td>%s</td><td>%d</td><td>%d</td>",
        (const char*)Socket::inetNtoA(ip), port,
        (int)_handle->getSocket().getFileDescriptor(),
        _handle->isSuspended() ? "suspended" : "active",
        _lastReadActivity ? (int)((Time::time() - _lastReadActivity) / 1000) : -1,
        (int)_handle->getSendBufferSize());
}
