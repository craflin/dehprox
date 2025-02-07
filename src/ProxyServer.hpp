
#pragma once

#include <nstd/Socket/Server.hpp>
#include <nstd/PoolList.hpp>

#include "Client.hpp"
#include "Settings.hpp"

class ProxyServer : public Server::Listener::ICallback
    , public Client::ICallback
{
public:
    ProxyServer(Settings& settings);

    bool start();
    bool startDebugPort();

    void run();

public: // Server::Listener::ICallback
    Server::Client::ICallback *onAccepted(Server::Client &client, uint32 ip, uint16 port) override;

public: // Client::ICallback
    void onClosed(Client& client) override;

private:
    class DebugListener;

    class DebugClient : public Server::Client::ICallback
    {
    public:
        DebugClient(DebugListener& parent, Server::Client& handle) : _parent(parent), _handle(handle) {}

    public: // Server::Client::ICallback
        void onRead() override;
        void onWrite() override {}
        void onClosed() override;

    private:
        DebugListener& _parent;
        Server::Client& _handle;
    };

    class DebugListener : public Server::Listener::ICallback
    {
    public:
        DebugListener(ProxyServer& parent) : _parent(parent) {}

    public: // Server::Listener::ICallback
        Server::Client::ICallback *onAccepted(Server::Client &client, uint32 ip, uint16 port) override;

    private:
        ProxyServer& _parent;
        PoolList<DebugClient> _debugClients;

        friend class DebugClient;
    };

private:
    Settings& _settings;
    Server _server;
    PoolList<::Client> _clients;
    DebugListener _debugListener;
};
