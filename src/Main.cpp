
#include <nstd/Log.h>
#include <nstd/Thread.h>
#include <nstd/Process.h>
#include <nstd/Console.h>
#include <nstd/Error.h>

#include "ProxyServer.h"
#include "DnsServer.h"

int main(int argc, char* argv[])
{
  String logFile;
  String configFile("/etc/dehprox.conf");

  // parse parameters
    {
        Process::Option options[] = {
            {'b', "daemon", Process::argumentFlag | Process::optionalFlag},
            {'c', "config", Process::argumentFlag},
            {'h', "help", Process::optionFlag},
        };
        Process::Arguments arguments(argc, argv, options);
        int character;
        String argument;
        while(arguments.read(character, argument))
            switch(character)
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
    DnsServer dnsServer(settings.dnsListenAddr);
    if (settings.dnsListenAddr.port)
    {
        if (!dnsServer.start())
            return Log::errorf("Could not start DNS server on UDP port %s:%hu: %s", (const char*)Socket::inetNtoA(settings.dnsListenAddr.addr), (uint16)settings.dnsListenAddr.port, (const char*)Socket::getErrorString()), 1;
        Log::infof("Listening on UDP port %hu...", (uint16)settings.dnsListenAddr.port);
    }

    // start transparent proxy server
    ProxyServer proxyServer(settings);
    if (!proxyServer.start())
        return Log::errorf("Could not start proxy server on TCP port %s:%hu: %s", (const char*)Socket::inetNtoA(settings.listenAddr.addr), (uint16)settings.listenAddr.port, (const char*)Socket::getErrorString()), 1;
    Log::infof("Listening on TCP port %hu...", (uint16)settings.listenAddr.port);

    // run dns server
    Thread dnsThread;
    if (settings.dnsListenAddr.port)
    {
        if (!dnsThread.start(dnsServer, &DnsServer::run))
            return Log::errorf("Could not start thread: %s", (const char*)Socket::getErrorString()), 1;
    }

    // run transparent proxy server
    proxyServer.run();
    return 1;
}
