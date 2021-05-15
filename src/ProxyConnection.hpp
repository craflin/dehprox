
#pragma once

#include <nstd/Socket/Server.hpp>

class ProxyConnection : public Server::Establisher::ICallback, public Server::Client::ICallback
{
public:
    class ICallback
    {
    public:
        virtual void onConnected(ProxyConnection&) = 0;
        virtual void onAbolished(ProxyConnection&) = 0;

    protected:
        ICallback() {}
        ~ICallback() {}
    };

public:
    ProxyConnection(Server& server, ICallback& callback);
    ~ProxyConnection();

    void setClientCallback(Server::Client::ICallback& callback) { _clientCallback = &callback;}

    bool connect();

    bool isConnected() {return _connected; }

    Server::Client& getHandle() {return *_handle;}

public: // Server::Establisher::ICallback
    Server::Client::ICallback *onConnected(Server::Client &client) override;
    void onAbolished() override;

public: // Server::Client::ICallback
    void onRead() override;
    void onWrite() override;
    void onClosed() override;

private:
    Server& _server;
    ICallback& _callback;
    Server::Client::ICallback* _clientCallback;
    Server::Establisher* _establisher;
    Server::Client* _handle;
    bool _connected;
};
