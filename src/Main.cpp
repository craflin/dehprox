
#include <nstd/Console.h>
#include <nstd/Error.h>
#include <nstd/Socket/Socket.h>

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

int main()
{
    Socket listen;
    if (!listen.open())
        return Console::errorf("Could not open socket: %s\n", (const char*)Error::getErrorString()), 1;
    
    if (!listen.bind(Socket::anyAddr, 62124))
        return Console::errorf("Could not bind socket: %s\n", (const char*)Error::getErrorString()), 1;

    if (!listen.listen())
        return Console::errorf("Could not listen on socket: %s\n", (const char*)Error::getErrorString()), 1;

    for (;;)
    {
        Socket s;
        uint32 ip;
        uint16 port;
        if (!listen.accept(s, ip, port))
            continue;

        Console::printf("accepted %s:%hu\n", (const char*)Socket::inetNtoA(ip), port);

        s.getSockName(ip, port);
        Console::printf("sockName=%s:%hu\n", (const char*)Socket::inetNtoA(ip), port);

        s.getPeerName(ip, port);
        Console::printf("peerName=%s:%hu\n", (const char*)Socket::inetNtoA(ip), port);

        getOriginalDst(s, ip, port);
        Console::printf("originalDst=%s:%hu\n", (const char*)Socket::inetNtoA(ip), port);
    }

    return 0;
}
