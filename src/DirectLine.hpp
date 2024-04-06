
#pragma once

#include "Address.hpp"

#include <nstd/Socket/Server.hpp>

class DirectLine : public Server::Establisher::ICallback, public Server::Client::ICallback
{
public:
    class ICallback
    {
    public:
        virtual void onOpened(DirectLine&) = 0;
        virtual void onClosed(DirectLine&, const String& error) = 0;

    protected:
        ICallback() {}
        ~ICallback() {}
    };

public:
    DirectLine(Server& server, Server::Client& client, ICallback& callback);
    ~DirectLine();

    Server::Client* getHandle() {return _handle;}

    bool connect(const Address& address);

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
    Server::Establisher* _establisher;
    Server::Client* _handle;
};
