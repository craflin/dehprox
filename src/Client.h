
#pragma once

#include "Uplink.h"

class ServerHandler;

class Client : public Connection
{
public:
    void connect(ServerHandler& server, Socket& client, uint32 ip, uint16 port);

    void onUplinkConnected();

public: // Connection
    virtual bool onRead();
    virtual bool onWrite();

public:
    ServerHandler* _server;
    Uplink _uplink;
};

