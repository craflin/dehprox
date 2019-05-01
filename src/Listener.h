
#pragma once

#include <nstd/Socket/Server.h>

#include "Client.h"
#include "Address.h"

class Listener : public Client::ICallback
{
public:
    Listener(Server& server, const Address& proxy);

    void accept(Server::Handle& listener);

public: // Client::ICallback
    virtual void onClosed(Client& client);

private:
    Server& _server;
    Address _proxy;
};
