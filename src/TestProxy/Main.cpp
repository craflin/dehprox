
#include <nstd/Socket/Server.hpp>
#include <nstd/Socket/Socket.hpp>
#include <nstd/Log.hpp>
#include <nstd/PoolList.hpp>
#include <nstd/Console.hpp>
#include <nstd/Buffer.hpp>

class Client : public Server::Client::ICallback, public Server::Establisher::ICallback
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
    Client(Server& server, Server::Client& client, ICallback& callback) : _server(server), _client(client), _callback(callback), _uplink(nullptr), _targetPort(0), _httpConnectMethod(false), _establisher(nullptr) {}
    ~Client();

public: // Server::Client::ICallback
    void onRead() override;
    void onWrite() override;
    void onClosed() override { _callback.onClosed(*this); }

private: // Server::Establisher::ICallback
    Server::Client::ICallback *onConnected(Server::Client &client) override;
    void onAbolished() override;

private:
    class Uplink : public Server::Client::ICallback
    {
    public:
        Server::Client& _uplink;

    public:
        Uplink(Client& client, Server::Client& uplink) : _uplink(uplink), _p(client) {}
        ~Uplink();

    private: // Server::Client::ICallback
        void onRead() override;
        void onWrite() override;
        void onClosed() override;

    private:
        Client& _p;
    };

private:
    Server& _server;
    Server::Client& _client;
    ICallback& _callback;
    Uplink* _uplink;
    String _target;
    uint16 _targetPort;
    bool _httpConnectMethod;
    Server::Establisher* _establisher;
    Buffer _receiveBuffer;
};

Client::~Client()
{
    _server.remove(_client);
    if (_establisher)
        _server.remove(*_establisher);
    delete _uplink;
}

Client::Uplink::~Uplink()
{
    _p._server.remove(_uplink);
}

namespace {

    bool parseNextWord(const char*& str, String& word)
    {
        while (String::isSpace(*str))
            ++str;
        const char* end = String::findOneOf(str, " \r\t\n");
        if (!end)
            return false;
        word.attach(str, end - str);
        str = end;
        return true;
    }

    bool parseSkipLine(const char*& str)
    {
        const char* end = String::findOneOf(str, "\r\n");
        if (!end)
            return false;
        str = end;
        if (*str == '\r')
            ++str;
        if (*str == '\n')
            ++str;
        return *str != '\r' && *str != '\n';
    }

    void parseHost(const String& uri, String& host, uint16& port)
    {
        const char* uriStart = uri;
        const char* portStart = String::find(uriStart, ':');
        if (portStart)
        {
            port = String::toInt(portStart + 1);
            host = uri.substr(0, portStart - uriStart);
        }
        else
        {
            host = uri;
            port = 0;
        }
    }

}

void Client::onRead()
{
    byte buffer[262144 + 1];
    usize size;
    if (!_client.read(buffer, sizeof(buffer) - 1, size))
        return;
    if (_uplink)
    {
        usize postponed = 0;
        if (!_uplink->_uplink.write(buffer, size, &postponed))
            return;
        if (postponed)
            _client.suspend();
    }
    else if (_establisher)
        _receiveBuffer.append(buffer, size);
    else
    {
        _receiveBuffer.append(buffer, size);
        const char* headerStart = (const char*)(const byte*)_receiveBuffer;
        const char* headerEnd = String::find(headerStart, "\r\n\r\n");
        if (headerEnd)
        {
            _client.suspend();

            const char* i = headerStart;
            String method;
            String uri;
            parseNextWord(i, method);
            parseNextWord(i, uri);
            {
                String headerField;
                while (parseSkipLine(i))
                {
                    if (!parseNextWord(i, headerField))
                        break;
                    if (headerField == "Host:")
                    {
                        String host;
                        parseNextWord(i, host);
                        parseHost(host, _target, _targetPort);
                        break;
                    }
                }
            }
            if (method == "CONNECT")
            {
                _httpConnectMethod = true;
                if (!_target.isEmpty())
                    parseHost(uri, _target, _targetPort);
                if (!_targetPort)
                    _targetPort = 80;
                _receiveBuffer.removeFront(headerEnd + 4 - headerStart);
            }
            else
            {
                int protocolLen = 0;
                uint16 defaultPort = 80;
                if (uri.startsWith("http://"))
                    protocolLen = 4;
                else if (uri.startsWith("https://"))
                {
                    protocolLen = 5;
                    defaultPort = 443;
                }
                if (!_targetPort)
                    _targetPort = defaultPort;
                String uriTarget;
                uint16 uriPort;
                if (protocolLen)
                {
                    const char* uriStart = (const char*)uri;
                    const char* hostStart = (const char*)uri + (protocolLen + 3);
                    const char* hostEnd = String::find(hostStart, "/");
                    if (!hostEnd)
                        hostEnd = (const char*)uri + uri.length();
                    String host;
                    host.attach(hostStart, hostEnd - hostStart);
                    parseHost(host, uriTarget, uriPort);
                    if (!uriPort)
                        uriPort = defaultPort;
                    if (_target.isEmpty())
                    {
                        _target = uriTarget;
                        _targetPort = uriPort;
                    }
                    //if (_target == uriTarget && _targetPort == uriPort)
                    //{
                    //    const char* uriSplit = String::find(headerStart + method.length(), "//");
                    //    if (uriSplit)
                    //    {
                    //        uriSplit = String::find(uriSplit + 2, '/');
                    //        if (uriSplit)
                    //        {
                    //            usize toRemove = uriSplit - headerStart - (method.length() + 1);
                    //            _receiveBuffer.removeFront(toRemove);
                    //            Memory::copy(_receiveBuffer, method, method.length());
                    //            ((char*)(byte*)_receiveBuffer)[method.length()] = ' ';
                    //            headerStart = (const char*)(const byte*)_receiveBuffer;
                    //        }
                    //    }
                    //}
                }
            }

            if (_target.isEmpty())
            {
                String request(headerStart, headerEnd - headerStart);
                Console::printf("Ignored request: %s\n", (const char*)request);
                _callback.onClosed(*this);
            }
            else
            {
                String request(headerStart, headerEnd - headerStart);
                Console::printf("Handled request: %s\n", (const char*)request);

                Console::printf("Connecting to %s:%d...\n", (const char*)_target, (int)_targetPort);
                _establisher = _server.connect(_target, _targetPort, *this);
            }

        }
        else if (_receiveBuffer.size() > 8 * 1024)
            _callback.onClosed(*this);
    }
}

void Client::onWrite()
{
    if (_uplink)
        _uplink->_uplink.resume();
}

void Client::Uplink::onRead()
{
    byte buffer[262144 + 1];
    usize size;
    if (!_uplink.read(buffer, sizeof(buffer) - 1, size))
        return;
    usize postponed;
    if (!_p._client.write(buffer, size, &postponed))
        return;
    if (postponed)
        _uplink.suspend();
}

void Client::Uplink::onWrite()
{
    _p._client.resume();
}

void Client::Uplink::onClosed()
{
    _p._callback.onClosed(_p);
}

Server::Client::ICallback *Client::onConnected(Server::Client &client)
{
    Console::printf("Connected to %s:%d\n", (const char*)_target, (int)_targetPort);
    if (_httpConnectMethod)
    {
        String response("HTTP/1.1 200 OK\r\n\r\n");
        if (!_client.write((const byte*)(const char*)response, response.length()))
            return nullptr;
    }
    _uplink = new Uplink(*this, client);
    usize postponed = 0;
    if (!_receiveBuffer.isEmpty())
    {
        if (!client.write(_receiveBuffer, _receiveBuffer.size(), &postponed))
            return nullptr;
        _receiveBuffer.clear();
    }
    if (!postponed)
        _client.resume();
    return _uplink;
}

void Client::onAbolished()
{
    _callback.onClosed(*this);
}

class Listener : public Server::Listener::ICallback, public Client::ICallback
{
public:
    Listener(Server& server) : _server(server) {}

public: // Server::Listener::ICallback
    Server::Client::ICallback* onAccepted(Server::Client& client, uint32 ip, uint16 port) override;

public:    // Client::ICallback
    void onClosed(Client& client) override;

private:
    Server& _server;
    PoolList<Client> _clients;
};

Server::Client::ICallback* Listener::onAccepted(Server::Client& client, uint32 ip, uint16 port)
{
    return &_clients.append<Server&, Server::Client&, Client::ICallback&>(_server, client, *this);
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

    if (!server.listen(Socket::anyAddress, port, listener))
        return Log::errorf("Could not start proxy server on TCP port %hu: %s", (uint16)port, (const char*)Socket::getErrorString()), 1;
    Log::infof("Listening on TCP port %hu...", (uint16)port);

    server.run();
    return 1;
}
