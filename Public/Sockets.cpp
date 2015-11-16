#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sstream>
#include "Sockets.h"
#include "Macro.h"
#include "Exception.h"

namespace Stone
{

int Socket::Create(int& error)
{
    _DBG("Socket::Create");
    //int sockfd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0)
    {
        error = errno;
        return -1;
    }
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(int));
    int nodelay = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    return sockfd;
}

bool Socket::Bind(int& sockfd, unsigned short port, int& error)
{
    _DBG("Socket::Bind port:%u", port);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        error = errno;
        Close(sockfd);
        return false;
    }

    _DBG("Socket::Bind succuss!");
    return true;
}

bool Socket::Listen(int sockfd, int& error)
{
    _DBG("Socket::Listen");
    int ret = listen(sockfd, SOMAXCONN);
    if (ret < 0)
    {
        error = errno;
        return false;
    }

    return true;
}

int Socket::Accept(int sockfd, struct sockaddr_in& addr, int& error)
{
    socklen_t len=sizeof(addr);
    int connfd = accept4(sockfd, (struct sockaddr*) &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0)
    {
        error = errno;
        switch (error)
        {
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            {
                std::stringstream sstream;
                sstream << "Socket: accept fd " << sockfd << "failed!";
                throwSystemError(sstream.str().c_str());
            }
            break;
        default:
            _ERR("Socket: accept fd %d failed : %d", sockfd, error);
            break;
        }
    }

    int nodelay = 1;
    if(setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) == -1)
    {
        _ERR("setsockopt fd(%d) no delay fail:%s", connfd, strerror(errno));
    }

    return connfd;
}

//to be changed
int Socket::Connect(int& sockfd, const std::string& dest, int& error)
{
    /*struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family=AF_UNIX;
    strncpy(addr.sun_path, dest.c_str(),sizeof(addr.sun_path)-1);

    int connfd = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if(connfd < 0)
    {
        error = errno;
        Close(sockfd);
    }

    return connfd;*/
    return -1;
}

void Socket::Close(int& sockfd)
{
    if(close(sockfd) < 0)
    {
        std::stringstream sstream;
        sstream << "Socket: close fd " << sockfd << "failed!";
        throwSystemError(sstream.str().c_str());
    }

    sockfd = -1;
}


struct sockaddr_in Socket::GetLocalAddr(int sockfd)
{
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if(getsockname(sockfd, (struct sockaddr *)(&localaddr), &addrlen) < 0)
    {
        _ERR("get (%d) local addr fail:%s", sockfd, strerror(errno));
    }

    return localaddr;
}

}

