#include <arpa/inet.h>
#include <event.h>
#include "TcpChannel.h"
#include "Sockets.h"
#include "Macro.h"

namespace Stone{

bool TcpChannel::Create()
{
    int error;
    int fd = Socket::Create(error);
    if(fd < 0)
    {
        _ERR("Create Socket fail!");
        return false;
    }

    if(!Socket::Bind(fd, Port_, error))
    {
        _ERR("Bind socket %d fail", fd);
        return false;
    }

    TcpIO_.reset(new IO(fd, EV_READ | EV_PERSIST));
    TcpIO_->SetCallback(std::bind(&TcpChannel::handleRead, this, std::placeholders::_1));

    return true;
}

bool TcpChannel::Listen()
{
    if(Listenning_)
    {
        _DBG("Tcp channel is already listenning.");
        return true;
    }

    int error;
    if(!Socket::Listen(TcpIO_->Fd(), error))
    {
        _ERR("Listen fail! Fd:%d, Error:%d", TcpIO_->Fd(), error);
        return false;
    }

    Listenning_ = true;
    return true;
}

void TcpChannel::handleRead(int cond)
{
    _DBG("begin tcp handle read...");
    struct sockaddr_in addr;
    int error;
    int fd = Socket::Accept(TcpIO_->Fd(), addr, error);
    if(fd < 0)
    {
        _ERR("Accept fail");
        if (error == EMFILE)
        {
            close(IdleFd_);
            IdleFd_ = accept(TcpIO_->Fd(), NULL, NULL);
            close(IdleFd_);
            IdleFd_ = open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
        return;
    }

    if(Callback_)
    {
        InetAddress iAddr;
        iAddr.addr = addr.sin_addr.s_addr;
        iAddr.saddr = inet_ntoa(addr.sin_addr);
        iAddr.port = addr.sin_port;
        Callback_(fd, iAddr);
    }
    else
    {
        close(fd);
    }
}

}
