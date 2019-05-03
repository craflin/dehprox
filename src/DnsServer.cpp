
#include "DnsServer.h"

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
#pragma pack(pop)

}

bool DnsServer::start(const Address& address)
{
    if (!_socket.open(Socket::udpProtocol) ||
        !_socket.bind(address.addr, address.port))
        return false;
    return true;
}

uint DnsServer::run()
{
    byte buffer[4096 * 2];
    Address sender;
    for (;;)
    {
        ssize size = _socket.recvFrom(buffer, sizeof(buffer), sender.addr, sender.port);
        if(size < 0)
            break;

        //??
    }
    return 1;
}
