
#pragma once

#include "Settings.hpp"

#include <nstd/String.hpp>
#include <nstd/Socket/Server.hpp>

class ProxyLine : public Server::Establisher::ICallback, public Server::Client::ICallback
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
    ProxyLine(Server& server, Server::Client& client, ICallback& callback, Settings& settings);
    ~ProxyLine();

    Server::Client* getHandle() {return _handle;}

    bool connect(const String& hostname, int16 port);

    String getDebugInfo() const;

public: // Server::Establisher::ICallback
    Server::Client::ICallback *onConnected(Server::Client &client) override;
    void onAbolished() override;

public: // Server::Client::ICallback
    void onRead() override;
    void onWrite() override;
    void onClosed() override;

private:
    Server& _server;
    Server::Client& _client;
    ICallback& _callback;
    Settings& _settings;
    Server::Establisher* _establisher;
    Server::Client* _handle;
    String _hostname;
    uint16 _port;
    Address _httpProxyAddr;
    bool _connected;
    String _proxyResponse;
    int64 _lastReadActivity;
};
