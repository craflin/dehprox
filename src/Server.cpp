
#include "Server.h"

#ifndef _WIN32
#include <arpa/inet.h>
#include <linux/netfilter_ipv4.h>
#endif

bool getOriginalDst(Socket& s, uint32& ip, uint16& port)
{
#ifdef _WIN32
    return s.getSockName(ip, port);
#else
   sockaddr_in destAddr;
    usize destAddrLen = sizeof(destAddr);
    if(!s.getSockOpt(SOL_IP, SO_ORIGINAL_DST, &destAddr, destAddrLen))
        return false;
    ip = ntohl(destAddr.sin_addr.s_addr);
    port = ntohs(destAddr.sin_port);
#endif
}

bool Server::onAccept(Socket& socket)
{
    Socket s;
    uint32 ip;
    uint16 port;
    if (!socket.accept(s, ip, port))
        return false;
    if (!getOriginalDst(s, ip, port))
        return false;
    Client& client = _clients.append();
    client.connect(*this, s, ip, port);
}

bool Server::poll()
{
    Socket::Poll::Event event;
    if (!_poll.poll(event, 60 * 1000))
        return false;
    if(!event.socket)
        return true; // timeout

    if (event.flags & Socket::Poll::acceptFlag && !onAccept(*event.socket))
        return true;
    if (event.flags & Socket::Poll::connectFlag && !((Uplink*)event.socket)->onConnected())
        return true;
    if (event.flags & Socket::Poll::readFlag && !((Connection*)event.socket)->onRead())
        return true;
    if (event.flags & Socket::Poll::writeFlag && !((Connection*)event.socket)->onWrite())
        return true;
}