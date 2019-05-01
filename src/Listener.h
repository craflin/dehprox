
#pragma once

#include <nstd/Socket/Server.h>

#include "Client.h"

class Listener : public Client::ICallback
{
public:
    Listener(Server& server);

    void accept(Server::Handle& handle);

public: // Client::ICallback
    virtual void onClosed(Client& client);

private:
    Server& _server;
};
