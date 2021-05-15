
#pragma once

#include "Settings.hpp"

#include <nstd/String.hpp>
#include <nstd/Socket/Server.hpp>

class ProxyConnection;

class ProxyLine : public Server::Client::ICallback
{
public:
    class ICallback
    {
    public:
        virtual void onConnected(ProxyLine&) = 0;
        virtual void onClosed(ProxyLine&, const String& error) = 0;

    protected:
        ICallback() {}
        ~ICallback() {}
    };

public:
    ProxyLine(Server& server, Server::Client& client,  ProxyConnection& proxy, ICallback& callback, const Settings& settings);

    Server::Client& getHandle() { return _handle; }
    ProxyConnection& getProxyConnection() { return _proxy; }

    bool connect(const String& hostname, uint16 port);

public: // Server::Client::ICallback
    void onRead() override;
    void onWrite() override;
    void onClosed() override;

private:
    Server& _server;
    Server::Client& _client;
    Server::Client& _handle;
    ICallback& _callback;
    const Settings& _settings;
    ProxyConnection& _proxy;
    bool _connected;
    String _proxyResponse;
};
