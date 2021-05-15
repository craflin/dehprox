
#include "ProxyServer.hpp"
#include "ProxyDatabase.hpp"

#include <nstd/Error.hpp>

ProxyServer::ProxyServer(const Settings& settings) : _settings(settings), _provisionedConnections(settings.server.connectionProvision | 1), _waitingClients(32)
{
    _server.setReuseAddress(true);
    _server.setKeepAlive(true);
    _server.setNoDelay(true);

    ProxyDatabase::add(settings.server.proxies, true);

    for (usize i = 0; i < settings.server.connectionProvision; ++i)
        addProxyConnection();
}

bool ProxyServer::start()
{
    if (!_server.listen(_settings.server.listenAddress.address, _settings.server.listenAddress.port, *this))
        return false;
    return true;
}

Server::Client::ICallback *ProxyServer::onAccepted(Server::Client &client_, uint32 ip, uint16 port)
{
    ::Client& client = _clients.append<Server&, Server::Client&, Client::ICallback&, const Settings&>(_server, client_, *this, _settings);
    if (!client.init())
    {
        _clients.remove(client);
        return nullptr;
    }

    usize remainingConnections = _settings.server.connectConcurrency;
    for (HashSet<ProxyConnection*>::Iterator i = _provisionedConnections.begin(), end = _provisionedConnections.end(); i != end; ++i)
    {
        ProxyConnection* proxy = *i;
        if (!proxy->isConnected())
            continue;
        _provisionedConnections.remove(i);
        if (!client.connect(*proxy))
        {
            _connections.remove(*proxy);
            continue;
        }
        if(--remainingConnections == 0)
            break;
    }

    if (remainingConnections)
        _waitingClients.append(&client, remainingConnections);

    usize connectionsToAdd = remainingConnections + _settings.server.connectionProvision - _provisionedConnections.size();
    for (usize i = 0; i < connectionsToAdd; ++i)
        addProxyConnection();

    return &client;
}

void ProxyServer::onEstablished(Client& client)
{
    _waitingClients.remove(&client);
}

void ProxyServer::onProxyFailed(Client& client)
{
    for (HashSet<ProxyConnection*>::Iterator i = _provisionedConnections.begin(), end = _provisionedConnections.end(); i != end; ++i)
    {
        ProxyConnection* proxy = *i;
        if (!proxy->isConnected())
            continue;
        _provisionedConnections.remove(i);
        if (!client.connect(*proxy))
        {
            _connections.remove(*proxy);
            continue;
        }
        addProxyConnection();
        return;
    }

    HashMap<::Client*, uint>::Iterator it = _waitingClients.find(&client);
    if (it == _waitingClients.end())
        _waitingClients.append(&client, 1);
    else
        ++*it;

    addProxyConnection();
}

void ProxyServer::onClosed(Client& client)
{
    _waitingClients.remove(&client);
    _clients.remove(client);

    if (_provisionedConnections.size() < _settings.server.connectionProvision)
        addProxyConnection();
}

void ProxyServer::addProxyConnection()
{
    ProxyConnection& connection = _connections.append<Server&, ProxyConnection::ICallback&>(_server, *this);
    if (!connection.connect())
        _connections.remove(connection);
}

void ProxyServer::onConnected(ProxyConnection& proxy)
{
    if (_waitingClients.isEmpty())
        return;

    _provisionedConnections.remove(&proxy);
    HashMap<::Client*, uint>::Iterator it = _waitingClients.begin();
    Client* client = it.key();
    if (!client->connect(proxy))
    {
        _connections.remove(proxy);
        addProxyConnection();
        return;
    }

    if (--*it == 0)
        _waitingClients.remove(it);
}

void ProxyServer::onAbolished(ProxyConnection& proxy)
{
    addProxyConnection();
    onClosed(proxy);
}

void ProxyServer::onClosed(ProxyConnection& proxy)
{
    _provisionedConnections.remove(&proxy);
    _connections.remove(proxy);

    if (_provisionedConnections.size() < _settings.server.connectionProvision)
        addProxyConnection();
}
