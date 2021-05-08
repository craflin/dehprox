
#include "Settings.hpp"

#include <nstd/File.hpp>
#include <nstd/List.hpp>
#include <nstd/Log.hpp>

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
        if (option == "server.proxy" || option == "httpProxyAddr")
            settings.server.httpProxyAddress.address = Socket::inetAddr(value, &settings.server.httpProxyAddress.port);
        else if (option == "server.listenAddress" || option == "listenAddr")
            settings.server.listenAddress.address = Socket::inetAddr(value, &settings.server.listenAddress.port);
        else if (option == "dns.listenAddress" || option == "dnsListenAddr")
            settings.dns.listenAddress.address = Socket::inetAddr(value, &settings.dns.listenAddress.port);
        else if (option == "server.autoProxySkip" || option == "autoProxySkip")
            settings.server.autoProxySkip = value.toBool();
        else if (option == "allow" || option == "allowDest")
            settings.whiteList.append(value);
        else if (option == "deny" || option == "denyDest")
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
