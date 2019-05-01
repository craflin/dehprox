
#pragma once

#include "Connection.h"

class Server;

class DirectLine : public Connection::ICallback
{
public:
    class ICallback
    {
    public:
        virtual void onOpened(DirectLine&) = 0;
        virtual void onClosed(DirectLine&) = 0;
        virtual void onAbolished(DirectLine&) = 0;

    protected:
        ICallback() {}
        ~ICallback() {}
    };

public:
    DirectLine(Server& server, Server::Handle& client, ICallback& callback);
    ~DirectLine();

    bool connect(uint32 addr, uint16 port);

    Server::Handle* getHandle() const {return _handle;}

public: // Connection::ICallback
    virtual void onOpened();
    virtual void onRead();
    virtual void onWrite();
    virtual void onClosed();
    virtual void onAbolished();

private:
    Server& _server;
    Server::Handle& _client;
    ICallback& _callback;
    Server::Handle* _handle;
};
