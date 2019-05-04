
#include <nstd/Log.h>
#include <nstd/Thread.h>

#include "ProxyServer.h"
#include "DnsServer.h"

int main()
{
    Address proxyUplink;
    proxyUplink.addr = Socket::loopbackAddr;
    proxyUplink.port = 3128;
    Address proxyListen;
    proxyListen.addr = Socket::anyAddr;
    proxyListen.port = 62124;
    Address dnsListen;
    dnsListen.addr = Socket::anyAddr;
    dnsListen.port = 62124;

    // start dns server
    DnsServer dnsServer;
    if (!dnsServer.start(dnsListen))
        return Log::errorf("Could not start DNS server on UDP port %s:%hu: %s", (const char*)Socket::inetNtoA(dnsListen.addr), (uint16)dnsListen.port, (const char*)Socket::getErrorString()), 1;
    Log::infof("Listening on UDP port %hu...", (uint16)dnsListen.port);

    // start transparent proxy server
    ProxyServer proxyServer;
    if (!proxyServer.start(proxyListen, proxyUplink))
        return Log::errorf("Could not start proxy server on TCP port %s:%hu: %s", (const char*)Socket::inetNtoA(proxyListen.addr), (uint16)proxyListen.port, (const char*)Socket::getErrorString()), 1;
    Log::infof("Listening on TCP port %hu...", (uint16)proxyListen.port);

    // run dns server
    Thread dnsThread;
    if (!dnsThread.start(dnsServer, &DnsServer::run))
        return Log::errorf("Could not start thread: %s", (const char*)Socket::getErrorString()), 1;

    // run transparent proxy server
    proxyServer.run();
    return 1;
}
