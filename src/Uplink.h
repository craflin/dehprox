
#pragma once

#include "Connection.h"

class Client;
class ServerHandler;

class Uplink : public Connection
{
public:
    void connect(ServerHandler& server, Client& client, uint32 ip, uint16 port);

    bool onConnected();

public: // Connection
    virtual bool onRead();
    virtual bool onWrite();

private:
    ServerHandler* _server;
    Client* _client;
};
