
#pragma once

#include <nstd/String.h>

#include "Connection.h"

class Server;

class ProxyLine : public Connection::ICallback
{
public:
    class ICallback
    {
    public:
        virtual void onOpened(ProxyLine&) = 0;
        virtual void onClosed(ProxyLine&) = 0;
        virtual void onAbolished(ProxyLine&) = 0;

    protected:
        ICallback() {}
        ~ICallback() {}
    };

public:
    ProxyLine(Server& server, Server::Handle& client, ICallback& callback);
    ~ProxyLine();

    bool connect(const String& hostname, int16 port);

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
    String _hostname;
};
