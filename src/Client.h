
#pragma once

#include <nstd/Socket/Server.hpp>

#include "Connection.h"
#include "DirectLine.h"
#include "ProxyLine.h"
#include "Settings.h"

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
    Client(Server& server, ICallback& callback, const Settings& settings);
    ~Client();

    bool accept(Server::Handle& listener);

public: // Connection::ICallback
    virtual void onOpened() {}
    virtual void onRead();
    virtual void onWrite();
    virtual void onClosed();
    virtual void onAbolished(uint) {}

public: // DirectLine::ICallback
    virtual void onOpened(DirectLine&);
    virtual void onClosed(DirectLine&, const String& error);

public: // ProxyLine::ICallback
    virtual void onOpened(ProxyLine&);
    virtual void onClosed(ProxyLine&, const String& error);

private:
    Server& _server;
    ICallback& _callback;
    const Settings& _settings;
    Server::Handle* _handle;
    ProxyLine* _proxyLine;
    DirectLine* _directLine;
    Server::Handle* _activeLine;
    Address _address;
    Address _destination;
    String _destinationHostname;

private:
    void close(const String& error);
};

