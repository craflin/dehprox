
#include "ProxyServer.hpp"

#include <nstd/Error.hpp>

ProxyServer::ProxyServer(const Settings& settings) : _settings(settings)
{
    _server.setReuseAddress(true);
    _server.setKeepAlive(true);
    _server.setNoDelay(true);
}

bool ProxyServer::start()
{
    if (!_server.listen(_settings.listenAddr.addr, _settings.listenAddr.port, *this))
        return false;
    return true;
}

void ProxyServer::run()
{
    _server.run();
}

Server::Client::ICallback *ProxyServer::onAccepted(Server::Client &client_, uint32 ip, uint16 port)
{
    Address address;
    address.addr = ip;
    address.port;

    ::Client& client = _clients.append<Server&, Server::Client&, const Address&, Client::ICallback&, const Settings&>(_server, client_, address, *this, _settings);
    if (!client.init())
    {
        _clients.remove(client);
        return nullptr;
    }
    return &client;
}

void ProxyServer::onClosed(Client& client)
{
    _clients.remove(client);
}
