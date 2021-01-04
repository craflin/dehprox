
#include <nstd/Socket/Server.hpp>
#include <nstd/Socket/Socket.hpp>
#include <nstd/Log.hpp>
#include <nstd/PoolList.hpp>

class Client : public Server::Client::ICallback
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
    Client() : _state(AcceptedState), _uplink(nullptr) {}

    void initialize(Server& server, Server::Client& client, ICallback& callback);

public: // Server::Client::ICallback
    void onRead() override;
    void onWrite() override {}
    void onClosed() override {_callback->onClosed(*this);}

private:
    enum State
    {
        AcceptedState,
    };

private:
    Server* _server;
    Server::Client* _client;
    Server::Client* _uplink;
    ICallback* _callback;
    State _state;
};

void Client::initialize(Server& server, Server::Client& client, ICallback& callback)
 {
    _server = &server;
    _client = &client;
    _callback = &callback;
 }

 void Client::onRead()
 {
    byte buffer[262144 + 1];
    usize size;
    if (!_client->read(buffer, sizeof(buffer) - 1, size))
        return;
    if (_uplink)
    {
        usize postponed = 0;
        if (!_uplink->write(buffer, size, &postponed))
            return;
        if (postponed)
            _client->suspend(); ?? server removes read flag from poll when out buffer is full. This is bad for full duplex
    }
    else
    {
        buffer[size] = '\0';
        _request.append((const char*)buffer, size);
        // expecting "HTTP/1.1 200 Connection established\r\n\r\n"
        const char* headerEnd = _request.find("\r\n\r\n");
        if (headerEnd)
        {
            ?? parse request
            if (_request.compare("HTTP/1.1 200 ", 13) == 0)
            {
                const char* bufferPos = headerEnd + 4;
                usize remainingSize = _proxyResponse.length() - (bufferPos - (const char*)_proxyResponse);
                if (remainingSize)
                    _server.write(_client, (const byte*)bufferPos, remainingSize);
                _proxyResponse = String();
                _connected = true;
                _callback.onOpened(*this);
            }
            else
                _callback.onClosed(*this, _proxyResponse.substr(0, firstLineEnd - (const char*)_proxyResponse));
        }
        else if(_request.length() > 256)
            _callback.onClosed(*this, "Invalid HTTP request");
    }
 }

class Listener : public Server::Listener::ICallback, public Client::ICallback
{
public:
    Listener(Server& server) : _server(server) {}

public: // Server::Listener::ICallback
    Server::Client::ICallback* onAccepted(Server::Client& client, uint32 ip, uint16 port) override;

public:  // Client::ICallback
    void onClosed(Client& client) override;

private:
    Server& _server;
    PoolList<Client> _clients;
};

Server::Client::ICallback* Listener::onAccepted(Server::Client& client_, uint32 ip, uint16 port)
{
    Client& client = _clients.append();
    client.initialize(_server, client_, *this);
    return &client;
}

void Listener::onClosed(Client& client)
{
    _clients.remove(client);
}

int main(int argc, char* argv[])
{
    uint16 port = 3128;

    Server server;
    server.setReuseAddress(true);
    server.setKeepAlive(true);
    server.setNoDelay(true);

    Listener listener(server);

    if (!server.listen(Socket::anyAddr, port, listener))
        return Log::errorf("Could not start proxy server on TCP port %hu: %s", (uint16)port, (const char*)Socket::getErrorString()), 1;
    Log::infof("Listening on TCP port %hu...", (uint16)port);

    server.run();
    return 1;
}
