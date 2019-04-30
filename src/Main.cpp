
#include <nstd/Console.h>
#include <nstd/Error.h>

#include "Server.h"

int main()
{
    Socket listener;
    if (!listener.open())
        return Console::errorf("Could not open socket: %s\n", (const char*)Error::getErrorString()), 1;
    if (!listener.setReuseAddress() ||
        !listener.setNonBlocking() ||
        !listener.bind(Socket::anyAddr, 62124))
        return Console::errorf("Could not bind socket: %s\n", (const char*)Error::getErrorString()), 1;
    if (!listener.listen())
        return Console::errorf("Could not listen on socket: %s\n", (const char*)Error::getErrorString()), 1;

    Server server;
    server.addListener(listener);

    for (;;)
        if (!server.poll())
            return 1;

    return 0;
}
