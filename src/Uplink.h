
#pragma once

#include "Connection.h"

class Client;
class Server;

class Uplink : public Connection
{
public:
    void connect(Server& server, Client& client, uint32 ip, uint16 port);

    bool onConnected();

public: // Connection
    virtual bool onRead();
    virtual bool onWrite();

private:
    Server* _server;
    Client* _client;
};
