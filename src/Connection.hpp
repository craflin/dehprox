
#pragma once

#include <nstd/Socket/Server.hpp>

class Connection
{
public:
    class ICallback
    {
    public:
        virtual void onOpened() = 0;
        virtual void onRead() = 0;
        virtual void onWrite() = 0;
        virtual void onClosed() = 0;
        virtual void onAbolished(uint error) = 0;

    protected:
        ICallback() {}
        ~ICallback() {}
    };
};
