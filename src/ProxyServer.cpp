
#include "ProxyServer.h"

ProxyServer::ProxyServer()
{
    _server.setReuseAddress(true);
    _server.setKeepAlive(true);
    _server.setNoDelay(true);
}

bool ProxyServer::start(const Address& address, const Address& proxy)
{
    _proxy = proxy;
    if (!_server.listen(address.addr, address.port, nullptr))
        return false;
    return true;
}

uint ProxyServer::run()
{
    for(Server::Event event; _server.poll(event);)
        switch(event.type)
        {
        case Server::Event::failType:
            ((Connection::ICallback*)event.userData)->onAbolished();
            break;
        case Server::Event::openType:
            ((Connection::ICallback*)event.userData)->onOpened();
            break;
        case Server::Event::readType:
            ((Connection::ICallback*)event.userData)->onRead();
            break;
        case Server::Event::writeType:
            ((Connection::ICallback*)event.userData)->onWrite();
            break;
        case Server::Event::closeType:
            ((Connection::ICallback*)event.userData)->onClosed();
            break;
        case Server::Event::acceptType:
            accept(*event.handle);
            break;
        }
    return 1;
}

void ProxyServer::accept(Server::Handle& listener)
{
    Client* client = new Client(_server, *this);
    if (!client->accept(_proxy, listener))
        delete client;
}

void ProxyServer::onClosed(Client& client)
{
    delete &client;
}
