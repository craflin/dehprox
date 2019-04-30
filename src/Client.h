
#pragma once

#include "Uplink.h"

class Server;

class Client : public Connection
{
public:
    void connect(Server& server, Socket& client, uint32 ip, uint16 port);

    void onUplinkConnected();

public: // Connection
    virtual bool onRead();
    virtual bool onWrite();

public:
    Server* _server;
    Uplink _uplink;
};

