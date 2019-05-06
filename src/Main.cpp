
#include <nstd/Log.h>
#include <nstd/Thread.h>

#include "ProxyServer.h"
#include "DnsServer.h"

int main()
{
    Settings settings;
    Settings::loadSettings("/etc/dehprox.conf" , settings);

    // start dns server
    DnsServer dnsServer(settings.dnsListenAddr);
    if (!dnsServer.start())
        return Log::errorf("Could not start DNS server on UDP port %s:%hu: %s", (const char*)Socket::inetNtoA(settings.dnsListenAddr.addr), (uint16)settings.dnsListenAddr.port, (const char*)Socket::getErrorString()), 1;
    Log::infof("Listening on UDP port %hu...", (uint16)settings.dnsListenAddr.port);

    // start transparent proxy server
    ProxyServer proxyServer(settings);
    if (!proxyServer.start())
        return Log::errorf("Could not start proxy server on TCP port %s:%hu: %s", (const char*)Socket::inetNtoA(settings.listenAddr.addr), (uint16)settings.listenAddr.port, (const char*)Socket::getErrorString()), 1;
    Log::infof("Listening on TCP port %hu...", (uint16)settings.listenAddr.port);

    // run dns server
    Thread dnsThread;
    if (!dnsThread.start(dnsServer, &DnsServer::run))
        return Log::errorf("Could not start thread: %s", (const char*)Socket::getErrorString()), 1;

    // run transparent proxy server
    proxyServer.run();
    return 1;
}
