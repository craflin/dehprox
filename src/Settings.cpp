
#include "Settings.h"

#include <nstd/File.hpp>
#include <nstd/List.hpp>
#include <nstd/Log.hpp>

Settings::Settings() : autoProxySkip(true)
{
    httpProxyAddr.addr = Socket::loopbackAddr;
    httpProxyAddr.port = 3128;
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
        if (option == "httpProxyAddr")
            settings.httpProxyAddr.addr = Socket::inetAddr(value, &settings.httpProxyAddr.port);
        else if (option == "listenAddr")
            settings.listenAddr.addr = Socket::inetAddr(value, &settings.listenAddr.port);
        else if (option == "dnsListenAddr")
            settings.dnsListenAddr.addr = Socket::inetAddr(value, &settings.dnsListenAddr.port);
        else if (option == "autoProxySkip")
            settings.autoProxySkip = value.toBool();
        else if (option == "allowDest")
            settings.whiteList.append(value);
        else if (option == "denyDest")
            settings.blackList.append(value);
        else
            Log::warningf("Unknown option: %s", (const char*)option);
    }
}

bool Settings::isInList(const String& hostname_, const HashSet<String>& list)
{
    if (list.contains(hostname_))
        return true;
    const char* x = hostname_.find('.');
    if (!x)
        return false;
    String hostname = hostname_.substr(x - (const char*)hostname_ + 1);
    for (;;)
    {
        if (list.contains(hostname))
            return true;
        const char* x = hostname.find('.');
        if (!x)
            return false;
        hostname = hostname.substr(x - (const char*)hostname + 1);
    }
}
