#pragma once

#include <nstd/Socket/Socket.h>

class Connection : public Socket
{
public:
    virtual bool onRead() = 0;
    virtual bool onWrite() = 0;
};
