
#pragma once

#include <nstd/Socket/Server.hpp>
#include <nstd/PoolList.hpp>

#include "Client.hpp"
#include "Settings.hpp"

class ProxyServer : public Server::Listener::ICallback
    , public Client::ICallback
{
public:
    ProxyServer(const Settings& settings);

    bool start();

    void run();

public: // Server::Listener::ICallback
    Server::Client::ICallback *onAccepted(Server::Client &client, uint32 ip, uint16 port) override;

public: // Client::ICallback
    void onClosed(Client& client) override;

private:
    const Settings& _settings;
    Server _server;
    PoolList<::Client> _clients;
};
