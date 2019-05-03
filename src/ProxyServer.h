
#pragma once

#include <nstd/Socket/Server.h>

#include "Client.h"
#include "Address.h"

class ProxyServer : public Client::ICallback
{
public:
    ProxyServer();

    bool start(const Address& address, const Address& proxy);

    uint run();

public: // Client::ICallback
    virtual void onClosed(Client& client);

private:
    Server _server;
    Address _proxy;

private:
    void accept(Server::Handle& listener);
};
