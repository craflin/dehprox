
#include "Settings.h"

#include <nstd/File.h>
#include <nstd/List.h>
#include <nstd/Log.h>

Settings::Settings() : autoProxySkip(true)
{
    proxyAddr.addr = Socket::loopbackAddr;
    proxyAddr.port = 3128;
    listenAddr.port = 62124;
    dnsListenAddr.port = 62124;
}

void Settings::loadSettings(const String& file, Settings& settings)
{
    String conf;
    if (!File::readAll(file, conf))
        return;
    List<String> lines;
    conf.split(lines, "\n\r");
    for (List<String>::Iterator i = lines.begin(), end = lines.end(); i != end; ++i)
    {
        String line = *i;
        const char* lineEnd = line.find('#');
        if (lineEnd)
            line.resize(lineEnd - (const char*)line);
        line.trim();
        if (line.isEmpty())
            continue;
        List<String> tokens;
        line.split(tokens, " \t");
        if (tokens.size() < 2)
            continue;
        const String& option = *tokens.begin();
        const String& value = *(++tokens.begin());
        if (option == "proxyAddr")
            settings.proxyAddr.addr = Socket::inetAddr(value, &settings.proxyAddr.port);
        else if (option == "listenAddr")
            settings.listenAddr.addr = Socket::inetAddr(value, &settings.listenAddr.port);
        else if (option == "dnsListenAddr")
            settings.dnsListenAddr.addr = Socket::inetAddr(value, &settings.dnsListenAddr.port);
        else if (option == "autoProxySkip")
            settings.autoProxySkip = value.toBool();
        else
            Log::warningf("Unknown option '%s'", (const char*)option);
    }
}

