
#pragma once

#include "Connection.hpp"
#include "Address.hpp"

class Server;

class DirectLine : public Connection::ICallback
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
    DirectLine(Server& server, Server::Handle& client, ICallback& callback);
    ~DirectLine();

    bool connect(const Address& address);

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
    Server::Handle* _handle;
};
