
#pragma once

#include <nstd/Socket/Server.hpp>

#include "DirectLine.hpp"
#include "ProxyLine.hpp"
#include "Settings.hpp"

class Client : public Server::Client::ICallback
    , public DirectLine::ICallback
    , public ProxyLine::ICallback
{
public:
    class ICallback
    {
    public:
        virtual void onClosed(Client& client) = 0;

    protected:
        ICallback() {}
        ~ICallback() {}
    };

public:
    Client(Server& server, Server::Client& client, ICallback& callback, const Settings& settings);
    ~Client();

    bool init();

public: // Server::Client::ICallback
    void onRead() override;
    void onWrite() override;
    void onClosed() override;

public: // DirectLine::ICallback
    void onOpened(DirectLine&) override;
    void onClosed(DirectLine&, const String& error) override;

public: // ProxyLine::ICallback
    void onOpened(ProxyLine&) override;
    void onClosed(ProxyLine&, const String& error) override;

private:
    Server& _server;
    Server::Client& _handle;
    ICallback& _callback;
    const Settings& _settings;
    ProxyLine* _proxyLine;
    DirectLine* _directLine;
    Server::Client* _activeLine;
    Address _address;
    Address _destination;
    String _destinationHostname;

private:
    void close(const String& error);
};

