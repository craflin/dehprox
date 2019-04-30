
#include "Client.h"
#include "Server.h"

void Client::connect(Server& server, Socket& client, uint32 ip, uint16 port)
{
    _server = &server;
    swap(client);
    setNonBlocking();
    _uplink.connect(server, *this, ip, port);
}

void Client::onUplinkConnected()
{
    _server->addConnection(*this);
}

bool Client::onRead()
{
    return true;
}

bool Client::onWrite()
{
    return true;
}
