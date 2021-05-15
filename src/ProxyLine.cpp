
#include "ProxyLine.hpp"
#include "ProxyDatabase.hpp"
#include "ProxyConnection.hpp"

#include <nstd/Error.hpp>

ProxyLine::ProxyLine(Server& server, Server::Client& client, ProxyConnection& proxy, ICallback& callback, const Settings& settings)
    : _server(server)
    , _client(client)
    , _handle(proxy.getHandle())
    , _callback(callback)
    , _settings(settings)
    , _proxy(proxy)
    , _connected(false)
{
    proxy.setClientCallback(*this);
}

bool ProxyLine::connect(const String& hostname, uint16 port)
{
    String connectMsg;
    connectMsg.printf("CONNECT %s:%hu HTTP/1.1\r\nHost: %s:%hu\r\n\r\n", (const char*)hostname, port, (const char*)hostname, port);
    if (!_handle.write((const byte*)(const char*)connectMsg, connectMsg.length()))
        return false;
    return true;
}

void ProxyLine::onRead()
{
    byte buffer[262144 + 1];
    usize size;
    if (!_handle.read(buffer, sizeof(buffer) - 1, size))
        return;
    if (_connected)
    {
        usize postponed = 0;
        if (!_client.write(buffer, size, &postponed))
            return;
        if (postponed)
            _handle.suspend();
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
                _callback.onConnected(*this);
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
