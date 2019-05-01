
#include "Listener.h"


#include <nstd/Socket/Socket.h>

#include "Client.h"

Listener::Listener(Server& server)
    : _server(server)
{
    ;
}

void Listener::accept(Server::Handle& handle)
{
    Client* client = new Client(_server, *this);
    if (!client->accept(handle))
        delete client;
}

void Listener::onClosed(Client& client)
{
    delete &client;
}
