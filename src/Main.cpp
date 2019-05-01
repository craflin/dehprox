
#include <nstd/Log.h>
#include <nstd/Socket/Socket.h>

#include "Listener.h"
#include "Connection.h"

int main()
{
    Address proxy;
    proxy.addr = Socket::loopbackAddr;
    proxy.port = 3129;
    Address listen;
    listen.addr = Socket::anyAddr;
    listen.port = 62124;


    Server server;
    server.setKeepAlive(true);
    server.setNoDelay(true);
    Listener listener(server, proxy);
    if(!server.listen(listen.addr, listen.port, nullptr))
    {
        Log::errorf("Could not listen on port %hu: %s", (uint16)listen.port, (const char*)Socket::getErrorString());
        return 1;
    }
    Log::infof("Listening on port %hu...", (uint16)listen.port);
    for(Server::Event event; server.poll(event);)
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
            listener.accept(*event.handle);
            break;
        }
    Log::errorf("Could not run poll loop: %s", (const char*)Socket::getErrorString());
    return 0;
}
