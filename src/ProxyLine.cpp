
#include "ProxyLine.hpp"

#include <nstd/Error.hpp>
#include <nstd/Time.hpp>

ProxyLine::ProxyLine(Server& server, Server::Client& client, ICallback& callback, const Address& httpProxyAddr)
    : _server(server)
    , _client(client)
    , _callback(callback)
    , _httpProxyAddr(httpProxyAddr)
    , _establisher(nullptr)
    , _handle(nullptr)
    , _port()
    , _connected(false)
    , _lastReadActivity(0)
{
    ;
}

ProxyLine::~ProxyLine()
{
    if(_establisher)
        _server.remove(*_establisher);
    if (_handle)
        _server.remove(*_handle);
}

bool ProxyLine::connect(const String& hostname, int16 port)
{
    _hostname = hostname;
    _port = port;
    _establisher = _server.connect(_httpProxyAddr.addr, _httpProxyAddr.port, *this);
    if (!_establisher)
        return false;
    return true;
}

Server::Client::ICallback *ProxyLine::onConnected(Server::Client &client)
{
    _handle = &client;
    String connectMsg;
    connectMsg.printf("CONNECT %s:%hu HTTP/1.1\r\nHost: %s:%hu\r\n\r\n", (const char*)_hostname, _port, (const char*)_hostname, _port);
    client.write((const byte*)(const char*)connectMsg, connectMsg.length());
    return this;
}

void ProxyLine::onAbolished()
{
    _callback.onClosed(*this, Error::getErrorString());
}

void ProxyLine::onRead()
{
    byte buffer[262144 + 1];
    usize size;
    if (!_handle->read(buffer, sizeof(buffer) - 1, size))
        return;
    if (_connected)
    {
        usize postponed = 0;
        if (!_client.write(buffer, size, &postponed))
            return;
        if (postponed)
            _handle->suspend();
        _lastReadActivity = Time::time();
    }
    else
    {
        buffer[size] = '\0';
        _proxyResponse.append((const char*)buffer, size);
        // expecting "HTTP/1.1 200 Connection established\r\n\r\n"
        const char* headerEnd = _proxyResponse.find("\r\n\r\n");
        if (headerEnd)
        {
            if (_proxyResponse.compare("HTTP/1.1 200 ", 13) == 0)
            {
                const char* bufferPos = headerEnd + 4;
                usize remainingSize = _proxyResponse.length() - (bufferPos - (const char*)_proxyResponse);
                if (remainingSize)
                    _client.write((const byte*)bufferPos, remainingSize);
                _proxyResponse = String();
                _connected = true;
                _callback.onOpened(*this);
                _lastReadActivity = Time::time();
            }
            else
            {
                const char* firstLineEnd = _proxyResponse.find("\r\n");
                _callback.onClosed(*this, _proxyResponse.substr(0, firstLineEnd - (const char*)_proxyResponse));
            }
        }
        else if (_proxyResponse.length() > 256)
            _callback.onClosed(*this, "Invalid HTTP reponse");
    }
}

void ProxyLine::onWrite()
{
    if (_connected)
        _client.resume();
}

void ProxyLine::onClosed()
{
    _callback.onClosed(*this, "Closed by proxy server");
}

String ProxyLine::getDebugInfo() const
{
    if (!_handle)
        return String("<td>connecting</td>");

    uint32 ip = 0;
    uint16 port = 0;
    _handle->getSocket().getSockName(ip, port);

    //<td>proxy</td><td>0.0.0.0:828</td><td>78</td><td>active</td><td>8</td><td>10.43.214.107:8080</td>
    return String::fromPrintf("<td>proxy</td><td>%s:%hu</td><td>%d</td><td>%s</td><td>%d</td><td>%d</td><td>%s:%hu</td>",
        (const char*)Socket::inetNtoA(ip), port,
        (int)_handle->getSocket().getFileDescriptor(),
        _handle->isSuspended() ? "suspended" : "active",
        _lastReadActivity ? (int)((Time::time() - _lastReadActivity) / 1000) : -1,
        (int)_handle->getSendBufferSize(),
        (const char*)Socket::inetNtoA(_httpProxyAddr.addr), _httpProxyAddr.port);
}
