
#pragma once

#include <nstd/Socket/Socket.h>
#include <nstd/PoolList.h>
#include "Client.h"

class Server
{
public:
    void addListener(Socket& socket) { _poll.set(socket, Socket::Poll::acceptFlag);}
    void addUplink(Socket& socket) { _poll.set(socket, Socket::Poll::connectFlag);}
    void addConnection(Socket& socket) { _poll.set(socket, Socket::Poll::readFlag);}

    bool poll();

private:
    Socket::Poll _poll;
    PoolList<Client> _clients;

private:
    bool onAccept(Socket& socket);
};
