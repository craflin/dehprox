
#include "Listener.h"


#include <nstd/Socket/Socket.h>

#include "Client.h"

Listener::Listener(Server& server, const Address& proxy)
    : _server(server)
    , _proxy(proxy)
{
    ;
}

void Listener::accept(Server::Handle& listener)
{
    Client* client = new Client(_server, *this);
    if (!client->accept(_proxy, listener))
        delete client;
}

void Listener::onClosed(Client& client)
{
    delete &client;
}
