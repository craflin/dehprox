
#include <nstd/Log.hpp>
#include <nstd/Thread.hpp>
#include <nstd/Process.hpp>
#include <nstd/Console.hpp>
#include <nstd/Error.hpp>

#include "ProxyServer.hpp"
#include "DnsServer.hpp"
#include "ProxyProvider.hpp"

int main(int argc, char* argv[])
{
    String logFile;
    String configFile("/etc/dehprox.conf");

    // parse parameters
    {
        Process::Option options[] = {
            { 'b', "daemon", Process::argumentFlag | Process::optionalFlag },
            { 'c', "config", Process::argumentFlag },
            { 'h', "help", Process::optionFlag },
        };
        Process::Arguments arguments(argc, argv, options);
        int character;
        String argument;
        while (arguments.read(character, argument))
            switch (character)
            {
            case 'b':
                logFile = argument.isEmpty() ? String("/dev/null") : argument;
                break;
            case 'c':
                configFile = argument;
                break;
            case '?':
                Console::errorf("Unknown option: %s.\n", (const char*)argument);
                return -1;
            case ':':
                Console::errorf("Option %s required an argument.\n", (const char*)argument);
                return -1;
            default:
                Console::errorf("Usage: %s [-b] [-c <file>]\n\
\n\
    -b, --daemon[=<file>]\n\
        Detach from calling shell and write output to <file>.\n\
\n\
    -c <file>, --config[=<file>]\n\
        Load configuration from <file>. (Default is /etc/dehprox.conf)\n\
\n", argv[0]);
                return -1;
            }
    }

    Log::setLevel(Log::debug);

    // load settings
    Settings settings;
    Settings::loadSettings(configFile, settings);

    // daemonize process
#ifndef _WIN32
    if(!logFile.isEmpty())
    {
        Log::infof("Starting as daemon...");
        if(!Process::daemonize(logFile))
        {
            Log::errorf("Could not daemonize process: %s", (const char*)Error::getErrorString());
            return -1;
        }
        Log::setDevice(Log::syslog);
        Log::setLevel(Log::info);
    }
#endif

    // start dns server
    DnsServer dnsServer(settings);
    if (settings.dns.listenAddress.port)
    {
        if (!dnsServer.start())
            return Log::errorf("Could not start DNS server on UDP port %s:%hu: %s", (const char*)Socket::inetNtoA(settings.dns.listenAddress.address), (uint16)settings.dns.listenAddress.port, (const char*)Socket::getErrorString()), 1;
        Log::infof("Listening on UDP port %hu...", (uint16)settings.dns.listenAddress.port);
    }

    // start transparent proxy server
    ProxyServer proxyServer(settings);
    if (!proxyServer.start())
        return Log::errorf("Could not start proxy server on TCP port %s:%hu: %s", (const char*)Socket::inetNtoA(settings.server.listenAddress.address), (uint16)settings.server.listenAddress.port, (const char*)Socket::getErrorString()), 1;
    Log::infof("Listening on TCP port %hu...", (uint16)settings.server.listenAddress.port);

    // run dns server
    Thread dnsThread;
    if (dnsServer.isStarted())
    {
        if (!dnsThread.start(dnsServer, &DnsServer::run))
            return Log::errorf("Could not start thread: %s", (const char*)Error::getErrorString()), 1;
    }

    // run proxy provider thread
    ProxyProvider proxyProvider(settings.provider);
    Thread providerThread;
    if (proxyProvider.isEnabled())
    {
        if (!providerThread.start(proxyProvider, &ProxyProvider::run))
            return Log::errorf("Could not start thread: %s", (const char*)Error::getErrorString()), 1;
    }

    // run transparent proxy server
    proxyServer.run();
    return 1;
}
