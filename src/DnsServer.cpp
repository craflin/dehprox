
#include "DnsServer.hpp"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

#include <cstring>

#include <nstd/Log.hpp>

#include "DnsDatabase.hpp"

namespace {

#pragma pack(push, 1)
    struct DnsHeader
    {
        uint16 id;
        uint16 flags;
        uint16 questionCount;
        uint16 answerCount;
        uint16 nameServerRecordCount;
        uint16 additionalRecordCount;
    };
    struct DnsQuestion
    {
        uint16 queryType;
        uint16 queryClass;
    };
    struct DnsAnswer
    {
        uint16 name;
        uint16 answerType;
        uint16 answerClass;
        uint32 validityTime;
        uint16 len;
        uint32 addr;
    };
#pragma pack(pop)

#define DNS_QR_BIT 0x80
#define DNS_RA_BIT 0x8000

    bool skipQuestion(const byte*& pos, const byte* end)
    {
        for (;;)
        {
            if (pos == end)
                return false;
            uint8 len = *(pos++);
            if (!len)
                break;
            if (pos + len > end)
                return false;
            pos += len;
        }
        if (pos + sizeof(DnsQuestion) > end)
            return false;
        pos += sizeof(DnsQuestion);
        return true;
    }

    bool parseQuestion(const byte*& pos, const byte* end, String& host, DnsQuestion& question)
    {
        host.clear();
        for (;;)
        {
            if (pos == end)
                return false;
            uint8 len = *(pos++);
            if (!len)
                break;
            if (pos + len > end)
                return false;
            if (!host.isEmpty())
                host.append('.');
            host.append((const char*)pos, len);
            pos += len;
        }
        if (pos + sizeof(DnsQuestion) > end)
            return false;
        question.queryType = ntohs(((const DnsQuestion*)pos)->queryType);
        question.queryClass = ntohs(((const DnsQuestion*)pos)->queryClass);
        pos += sizeof(DnsQuestion);
        return true;
    }

    bool appendAnswer(const byte*& pos, const byte* end, const DnsQuestion& question, usize offset, uint32 addr)
    {
        if (pos + sizeof(DnsAnswer) > end)
            return false;
        DnsAnswer* answer = (DnsAnswer*)pos;
        answer->name = htons(offset | 0xc000);
        answer->answerType = htons(question.queryType);
        answer->answerClass = htons(question.queryClass);
        answer->validityTime = htonl(10 * 60); // 10 minutes
        answer->len = htons(sizeof(uint32));
        answer->addr = htonl(addr);
        pos += sizeof(DnsAnswer);
        return true;
    }

}

bool DnsServer::start()
{
    if (!_socket.open(Socket::udpProtocol) ||
        !_socket.setReuseAddress() ||
        !_socket.bind(_settings.dns.listenAddress.address, _settings.dns.listenAddress.port))
        return false;
    return true;
}

uint DnsServer::run()
{
    byte query[4096 * 2];
    byte response[4096 * 2];
    Address sender;
    String hostname;
    DnsHeader* queryHeader = (DnsHeader*)query;
    DnsHeader* responseHeader = (DnsHeader*)response;
    const byte* responseEnd = response + sizeof(response);
    for (;;)
    {
        ssize size = _socket.recvFrom(query, sizeof(query), sender.address, sender.port);
        if (size < 0)
            break;
        if (size < sizeof(DnsHeader))
            continue;
        uint16 flags = ntohs(queryHeader->flags);
        if (flags & DNS_QR_BIT)
            continue;
        uint16 questionCount = ntohs(queryHeader->questionCount);
        const byte* pos = (const byte*)(queryHeader + 1);
        const byte* end = query + size;
        for (uint16 i = 0; i < questionCount; ++i)
            if (!skipQuestion(pos, end))
                goto ignoreRequest;
        {
            usize querySize = end - query;
            const byte* responsePos = response + querySize;
            pos = (const byte*)(queryHeader + 1);
            uint16 answerCount = 0;
            for (uint16 i = 0; i < questionCount; ++i)
            {
                const byte* pointerPos = pos;
                DnsQuestion question;
                if (!parseQuestion(pos, end, hostname, question))
                    goto ignoreRequest;

                const char* rejectReason = nullptr;
                if (!_settings.whiteList.isEmpty() && !Settings::isInList(hostname, _settings.whiteList))
                    rejectReason = "Not listed in white list";
                else if (Settings::isInList(hostname, _settings.blackList))
                    rejectReason = "Listed in black list";

                if (rejectReason)
                {
                    Log::infof("%s: Ignored DNS query for %s: %s", (const char*)Socket::inetNtoA(sender.address), (const char*)hostname, rejectReason);
                    continue; // don't try to resolve black listed hostnames to keep them out of the DnsDatabase
                }

                uint32 addr;
                bool isFakeAddr = false;
                if (!_settings.dns.resolveAddresses || !DnsDatabase::resolve(hostname, addr))
                {
                    if (!hostname.find('.'))
                    {
                        Log::debugf("%s: Ignored DNS query for %s", (const char*)Socket::inetNtoA(sender.address), (const char*)hostname);
                        continue;
                    }
                    addr = DnsDatabase::resolveFake(hostname);
                    isFakeAddr = true;
                }

                Log::debugf("%s: Answered DNS query for %s with %s%s", (const char*)Socket::inetNtoA(sender.address),
                    (const char*)hostname, (const char*)Socket::inetNtoA(addr), isFakeAddr ? " (surrogate)" : "");

                if (!appendAnswer(responsePos, responseEnd, question, pointerPos - query, addr))
                    goto ignoreRequest;
                ++answerCount;
            }
            // create response header
            memcpy(responseHeader, query, querySize);
            responseHeader->flags = htons(flags | DNS_QR_BIT | DNS_RA_BIT);
            responseHeader->answerCount = htons(answerCount);
            responseHeader->nameServerRecordCount = 0;
            responseHeader->additionalRecordCount = 0;

            // send response
            _socket.sendTo((byte*)responseHeader, responsePos - (byte*)responseHeader, sender.address, sender.port);
        }
    ignoreRequest:;
    }
    return 1;
}
