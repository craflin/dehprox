
#include "ProxyServer.hpp"

#include <nstd/Error.hpp>

ProxyServer::ProxyServer(const Settings& settings) : _settings(settings) , _debugListener(*this)
{
    _server.setReuseAddress(true);
    _server.setKeepAlive(true);
    _server.setNoDelay(true);
}

bool ProxyServer::start()
{
    const Address& listenAddr = _settings.getListenAddr();
    if (!_server.listen(listenAddr.addr, listenAddr.port, *this))
        return false;
    return true;
}


bool ProxyServer::startDebugPort()
{
    if (_settings.debugListenAddr.port)
        if (!_server.listen(_settings.debugListenAddr.addr, _settings.debugListenAddr.port, _debugListener))
            return false;
    return true;
}

void ProxyServer::run()
{
    _server.run();
}

Server::Client::ICallback *ProxyServer::onAccepted(Server::Client &client_, uint32 ip, uint16 port)
{
    Address address;
    address.addr = ip;
    address.port = port;

    ::Client& client = _clients.append<Server&, Server::Client&, const Address&, Client::ICallback&, const Settings&>(_server, client_, address, *this, _settings);
    if (!client.init())
    {
        _clients.remove(client);
        return nullptr;
    }
    return &client;
}

void ProxyServer::onClosed(Client& client)
{
    _clients.remove(client);
}

Server::Client::ICallback *ProxyServer::DebugListener::onAccepted(Server::Client &client, uint32, uint16)
{
    return &_debugClients.append<DebugListener&, Server::Client&>(*this, client);
}

void ProxyServer::DebugClient::onRead()
{
    byte buffer[262144];
    usize size;
    if (!_handle.read(buffer, sizeof(buffer), size))
        return;

    String response;
    response.append("<html>\n");
    response.append("<head><style>table, th, td { border: 1px solid black; border-collapse: collapse;}</style></head>\n");
    response.append("<body>\n");
    response.append("<p>\n");
    response.append("<table>\n");

    response.append("<tr><th>client</th><th>fd</th><th>poll</th><th>idle</th><th>sndbuf</th><th>destination</th><th>mode</th><th>sock</th><th>fd</th><th>poll</th><th>idle</th><th>sndbuf</th><th>proxy</th></tr>");

    for (PoolList<::Client>::Iterator i = _parent._parent._clients.begin(), end =_parent._parent._clients.end(); i != end; ++i)
    {
        const ::Client& client = *i;
        response.append(client.getDebugInfo());
        response.append("\n");
    }

    response.append("</table>\n");
    response.append("</p>\n");
    response.append("</body>\n");
    response.append("</html>\n");
    String header = String::fromPrintf("HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n", (int)response.length());
    _handle.write((const byte*)(const char*)header, header.length());
    _handle.write((const byte*)(const char*)response, response.length());
}

void ProxyServer::DebugClient::onClosed()
{
    _parent._parent._server.remove(_handle);
    _parent._debugClients.remove(*this);
}
