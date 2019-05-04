
#pragma once

#include <nstd/Socket/Server.h>

#include "Connection.h"
#include "DirectLine.h"
#include "ProxyLine.h"
#include "Address.h"

class Client : public Connection::ICallback
             , public DirectLine::ICallback
             , public ProxyLine::ICallback
{
public:
    class ICallback
    {;
    public:
        virtual void onClosed(Client& client) = 0;

    protected:
        ICallback() {}
        ~ICallback() {}
    };

public:
    Client(Server& server, ICallback& callback);
    ~Client();

    bool accept(const Address& proxy, Server::Handle& listener);

public: // Connection::ICallback
    virtual void onOpened() {}
    virtual void onRead();
    virtual void onWrite();
    virtual void onClosed();
    virtual void onAbolished() {}

public: // DirectLine::ICallback
    virtual void onOpened(DirectLine&);
    virtual void onClosed(DirectLine&);

public: // ProxyLine::ICallback
    virtual void onOpened(ProxyLine&);
    virtual void onClosed(ProxyLine&);

public:
    Server& _server;
    ICallback& _callback;
    Server::Handle* _handle;
    ProxyLine* _proxyLine;
    DirectLine* _directLine;
    Server::Handle* _activeLine;
    Address _address;
    Address _destination;
    String _destinationHostname;
};

