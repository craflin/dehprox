
#pragma once

#include <nstd/Socket/Server.hpp>
#include <nstd/PoolList.hpp>

#include "DirectLine.hpp"
#include "ProxyLine.hpp"
#include "Settings.hpp"

class ProxyConnection;

class Client : public Server::Client::ICallback
    , public DirectLine::ICallback
    , public ProxyLine::ICallback
{
public:
    class ICallback
    {
    public:
        virtual void onClosed(ProxyConnection& proxy) = 0;
        virtual void onClosed(Client& client) = 0;
        virtual void onEstablished(Client& client) = 0;
        virtual void onProxyFailed(Client& client) = 0;

    protected:
        ICallback() {}
        ~ICallback() {}
    };

public:
    Client(Server& server, Server::Client& client, Address& address, ICallback& callback, const Settings& settings);
    ~Client();

    bool init();
    bool connect(ProxyConnection& proxy);

public: // Server::Client::ICallback
    void onRead() override;
    void onWrite() override;
    void onClosed() override;

public: // DirectLine::ICallback
    void onConnected(DirectLine&) override;
    void onClosed(DirectLine&, const String& error) override;

public: // ProxyLine::ICallback
    void onConnected(ProxyLine&) override;
    void onClosed(ProxyLine&, const String& error) override;

private:
    Server& _server;
    Server::Client& _handle;
    const Address _address;
    ICallback& _callback;
    const Settings& _settings;
    PoolList<ProxyLine> _proxyLines;
    DirectLine* _directLine;
    Server::Client* _activeLine;
    Address _destination;
    String _destinationHostname;
    uint _failedProxyConnections;

private:
    void close(const String& error);
};

