#ifndef _STONE_INET_ADDRESS_H_
#define _STONE_INET_ADDRESS_H_

#include <string>
#include <unistd.h>

namespace Stone{

struct InetAddress
{
    uint32_t addr;
    std::string saddr;
    uint16_t port;

    InetAddress() : addr(0), port(0){}
    InetAddress(const InetAddress& right):
        addr(right.addr),
        saddr(right.saddr),
        port(right.port){
        }
    InetAddress(InetAddress&& right):
        addr(right.addr),
        saddr(std::move(right.saddr)),
        port(right.port){
            right.addr = 0;
            right.port = 0;
        }

    InetAddress& operator= (const InetAddress& right)
    {
        if(&right == this)
        {
            return *this;
        }

        addr = right.addr;
        saddr = right.saddr;
        port = right.port;

        return *this;
    }

    InetAddress& operator= (InetAddress&& right)
    {
        if(&right == this)
        {
            return *this;
        }

        addr = right.addr;
        saddr = std::move(right.saddr);
        port = right.port;
        right.addr = 0;
        right.port = 0;

        return *this;
    }
};

};

#endif
