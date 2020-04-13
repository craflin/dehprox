
#pragma once

#include <nstd/Socket/Server.hpp>

#include "Client.hpp"
#include "Address.hpp"
#include "Settings.hpp"

class ProxyServer : public Client::ICallback
{
public:
    ProxyServer(const Settings& settings);

    bool start();

    uint run();

public: // Client::ICallback
    virtual void onClosed(Client& client);

private:
    const Settings& _settings;
    Server _server;

private:
    void accept(Server::Handle& listener);
};
