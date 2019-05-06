
#include "ProxyServer.h"

ProxyServer::ProxyServer(const Settings& settings) : _settings(settings)
{
    _server.setReuseAddress(true);
    _server.setKeepAlive(true);
    _server.setNoDelay(true);
}

bool ProxyServer::start()
{
    if (!_server.listen(_settings.listenAddr.addr, _settings.listenAddr.port, nullptr))
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
    Client* client = new Client(_server, *this, _settings);
    if (!client->accept(listener))
        delete client;
}

void ProxyServer::onClosed(Client& client)
{
    delete &client;
}
