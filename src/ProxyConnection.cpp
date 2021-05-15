
#include "ProxyConnection.hpp"
#include "ProxyDatabase.hpp"

#include <nstd/Error.hpp>

ProxyConnection::ProxyConnection(Server& server, ICallback& callback) 
    : _server(server)
    , _callback(callback)
    , _clientCallback(nullptr)
    , _establisher(nullptr)
    , _handle(nullptr)
    , _connected(false)
{
    ;
}

ProxyConnection::~ProxyConnection()
{
    if(_establisher)
        _server.remove(*_establisher);
    if (_handle)
        _server.remove(*_handle);
}

bool ProxyConnection::connect()
{
    Address proxyAddress;
    if (!ProxyDatabase::getRandom(proxyAddress))
        return false;

    _establisher = _server.connect(proxyAddress.address, proxyAddress.port, *this);
    if (!_establisher)
        return false;
    return true;
}

Server::Client::ICallback* ProxyConnection::onConnected(Server::Client &client)
{
    _handle = &client;
    _connected = true;
    _callback.onConnected(*this);
    return this;
}

void ProxyConnection::onAbolished() 
{
    _callback.onAbolished(*this);
}

void ProxyConnection::onRead()
{
    if (_clientCallback)
        _clientCallback->onRead();
    else
        _callback.onAbolished(*this);
}

void ProxyConnection::onWrite()
{
    if (_clientCallback)
        _clientCallback->onWrite();
}

void ProxyConnection::onClosed()
{
    if (_clientCallback)
        _clientCallback->onClosed();
    else
        _callback.onAbolished(*this);
}
