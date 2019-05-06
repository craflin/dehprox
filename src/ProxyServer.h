
#pragma once

#include <nstd/Socket/Server.h>

#include "Client.h"
#include "Address.h"
#include "Settings.h"

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
