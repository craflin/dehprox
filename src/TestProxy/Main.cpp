
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
    Client(Server& server, Server::Client& client, ICallback& callback) : _server(server), _client(client), _callback(callback), _uplink(nullptr), _establisher(nullptr) {}
    ~Client();

public: // Server::Client::ICallback
    void onRead() override;
    void onWrite() override;
    void onClosed() override {_callback.onClosed(*this);}

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

String parseNextWord(const char*& str)
{
  while(String::isSpace(*str))
    ++str;
  const char* end = String::findOneOf(str, " \r\t\n");
  if (!end)
    return String();
  String result(str, end - str);
  str = end + 1;
  return result;
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
          const char* i = headerStart;
          String method = parseNextWord(i);
          String target = parseNextWord(i);
          if (method == "CONNECT")
          {
            uint16 port = 80;
            const char* targetStart = target;
            const char* portStart = String::find(targetStart, ':');
            if (portStart)
            {
              port = String::toInt(portStart + 1);
              target.resize(portStart - targetStart);
            }
            _establisher = _server.connect(target, port, *this);
            _receiveBuffer.removeFront(headerEnd + 4 - headerStart);
            _client.suspend();
          }
          //else
          //{
          //  // todo: support "GET", "POST", etc
          //  const char* lineEnd = String::find(headerStart, "\r\n") + 2;
          //  String line;
          //  while(parseNextLine(lineEnd, line))
          //  {
          //    if (!parseNextHeaderField(line))
          //  }
          //  ??
          //}
          else
          {
            String request(headerStart, headerEnd - headerStart);
            Console::printf("Ignored request: %s\n", (const char*)request);
            _callback.onClosed(*this);
          }
        }
        else if(_receiveBuffer.size() > 1024)
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
    if(postponed)
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
  Console::printf("connected\n");
  _uplink = new Uplink(*this, client);
  usize postponed;
  if (!client.write(_receiveBuffer, _receiveBuffer.size(), &postponed))
    return nullptr;
  _receiveBuffer.clear();
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

public:  // Client::ICallback
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

    if (!server.listen(Socket::anyAddr, port, listener))
        return Log::errorf("Could not start proxy server on TCP port %hu: %s", (uint16)port, (const char*)Socket::getErrorString()), 1;
    Log::infof("Listening on TCP port %hu...", (uint16)port);

    server.run();
    return 1;
}
