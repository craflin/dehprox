
#include <nstd/Console.h>
#include <nstd/Error.h>
#include <nstd/Socket/Socket.h>

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
        Socket x;
        uint32 ip;
        uint16 port;
        if (!listen.accept(x, ip, port))
            continue;

        Console::printf("accepted %s:%hu\n", (const char*)Socket::inetNtoA(ip), port);

        x.getSockName(ip, port);
        Console::printf("sockName=%s:%hu\n", (const char*)Socket::inetNtoA(ip), port);

        x.getPeerName(ip, port);

        Console::printf("peerName=%s:%hu\n", (const char*)Socket::inetNtoA(ip), port);
    }

    return 0;
}
