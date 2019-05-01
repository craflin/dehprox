
#include "ProxyLine.h"

ProxyLine::ProxyLine(Server& server, Server::Handle& client, ICallback& callback)
    : _server(server)
    , _client(client)
    , _callback(callback)
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

bool ProxyLine::connect(const Address& proxy, const String& hostname, int16 port)
{
    _hostname = hostname;
    _port = port;
    _handle = _server.connect(proxy.addr, proxy.port, this);
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
        const char* headerEnd = String::find((const char*)buffer, "\r\n\r\n");
        if (headerEnd)
        {
            if (String::compare((const char*)buffer, "200 OK", 6) == 0)
            {
                const byte* bufferPos = (const byte*)headerEnd + 4;
                usize remainingSize = size - (bufferPos - buffer);
                _connected = true;
                _callback.onOpened(*this);
                if (remainingSize)
                    _server.write(_client, bufferPos, remainingSize);
            }
            else
                _callback.onClosed(*this);
        }
        else
        {
            //if(size > 128)
                _callback.onClosed(*this);
            //else
                //_server.unread(*_handle, buffer, size); // todo
        }
    }
}

void ProxyLine::onWrite()
{
    if (_connected)
        _server.resume(_client);
}

void ProxyLine::onClosed()
{
    _callback.onClosed(*this);
}

void ProxyLine::onAbolished()
{
    _callback.onClosed(*this);
}
