
#pragma once

#include <nstd/String.hpp>

#include "Connection.h"
#include "Settings.h"

class Server;

class ProxyLine : public Connection::ICallback
{
public:
    class ICallback
    {
    public:
        virtual void onOpened(ProxyLine&) = 0;
        virtual void onClosed(ProxyLine&, const String& error) = 0;

    protected:
        ICallback() {}
        ~ICallback() {}
    };

public:
    ProxyLine(Server& server, Server::Handle& client, ICallback& callback, const Settings& settings);
    ~ProxyLine();

    bool connect(const String& hostname, int16 port);

    Server::Handle* getHandle() const {return _handle;}

public: // Connection::ICallback
    virtual void onOpened();
    virtual void onRead();
    virtual void onWrite();
    virtual void onClosed();
    virtual void onAbolished(uint error);

private:
    Server& _server;
    Server::Handle& _client;
    ICallback& _callback;
    const Settings& _settings;
    Server::Handle* _handle;
    String _hostname;
    uint16 _port;
    bool _connected;
    String _proxyResponse;
};
