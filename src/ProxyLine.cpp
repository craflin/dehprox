
#include "ProxyLine.hpp"

#include <nstd/Error.hpp>

ProxyLine::ProxyLine(Server& server, Server::Handle& client, ICallback& callback, const Settings& settings)
    : _server(server)
    , _client(client)
    , _callback(callback)
    , _settings(settings)
    , _handle(nullptr)
    , _port()
    , _connected(false)
{
    ;
}

ProxyLine::~ProxyLine()
{
    if (_handle)
        _server.close(*_handle);
}

bool ProxyLine::connect(const String& hostname, int16 port)
{
    _hostname = hostname;
    _port = port;
    _handle = _server.connect(_settings.httpProxyAddr.addr, _settings.httpProxyAddr.port, this);
    if (!_handle)
        return false;
    return true;
}

void ProxyLine::onOpened()
{
    String connectMsg;
    connectMsg.printf("CONNECT %s:%hu HTTP/1.1\r\nHost: %s:%hu\r\n\r\n", (const char*)_hostname, _port, (const char*)_hostname, _port);
    _server.write(*_handle, (const byte*)(const char*)connectMsg, connectMsg.length());
}

void ProxyLine::onRead()
{
    byte buffer[262144 + 1];
    usize size;
    if (!_server.read(*_handle, buffer, sizeof(buffer) - 1, size))
        return;
    if (_connected)
    {
        usize postponed = 0;
        if (!_server.write(_client, buffer, size, &postponed))
            return;
        if (postponed)
            _server.suspend(*_handle);
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
                    _server.write(_client, (const byte*)bufferPos, remainingSize);
                _proxyResponse = String();
                _connected = true;
                _callback.onOpened(*this);
            }
            else
            {
                const char* firstLineEnd = _proxyResponse.find("\r\n");
                _callback.onClosed(*this, _proxyResponse.substr(0, firstLineEnd - (const char*)_proxyResponse));
            }
        }
        else if(_proxyResponse.length() > 256)
            _callback.onClosed(*this, "Invalid HTTP reponse");
    }
}

void ProxyLine::onWrite()
{
    if (_connected)
        _server.resume(_client);
}

void ProxyLine::onClosed()
{
    _callback.onClosed(*this, "Closed by proxy server");
}

void ProxyLine::onAbolished(uint error)
{
    _callback.onClosed(*this, Error::getErrorString(error));
}
