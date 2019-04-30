
#include "Uplink.h"
#include "Server.h"

void Uplink::connect(Server& server, Client& client, uint32 ip, uint16 port)
{
    _server = &server;
    _client = &client;

    open();
    setNonBlocking();
    Socket::connect(ip, port);
    _server->addUplink(*this);
}

bool Uplink::onConnected()
{
    _server->addConnection(*this);
    _client->onUplinkConnected();
    return true;
}

bool Uplink::onRead()
{
    return true;
}

bool Uplink::onWrite()
{
    return true;
}
