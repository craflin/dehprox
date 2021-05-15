
#pragma once

#include <nstd/Socket/Server.hpp>
#include <nstd/PoolList.hpp>
#include <nstd/HashSet.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/List.hpp>

#include "Client.hpp"
#include "Settings.hpp"
#include "ProxyConnection.hpp"

class ProxyServer : public Server::Listener::ICallback
    , public Client::ICallback
    , public ProxyConnection::ICallback
{
public:
    ProxyServer(const Settings& settings);

    bool start();

    void run() { _server.run(); }

public: // Server::Listener::ICallback
    Server::Client::ICallback *onAccepted(Server::Client &client, uint32 ip, uint16 port) override;

public: // Client::ICallback
    void onEstablished(Client& client) override;
    void onProxyFailed(Client& client) override;
    void onClosed(Client& client) override;
    void onClosed(ProxyConnection& proxy) override;

public: // ProxyConnection::ICallback
    void onConnected(ProxyConnection&) override;
    void onAbolished(ProxyConnection&) override;

private:
    const Settings& _settings;
    Server _server;
    PoolList<::Client> _clients;

    PoolList<ProxyConnection> _connections;
    HashSet<ProxyConnection*> _provisionedConnections;
    HashMap<::Client*, uint> _waitingClients;

private:
    void addProxyConnection();
};
