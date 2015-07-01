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
	int sockfd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (sockfd < 0)
	{
		error = errno;
		return -1;
	}

	return sockfd;
}

bool Socket::Bind(int& sockfd, const std::string& path, int& error)
{
	struct sockaddr_un addr;
	memset(addr, 0, sizeof(addr));
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path, path.c_str(),sizeof(addr.sun_path)-1);

	if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
	{
		error = errno;
		Close(sockfd);
		return false;
	}

	return true;
}

bool Socket::Listen(int sockfd, int& error)
{
	int ret = listen(sockfd, SOMAXCONN);
	if (ret < 0)
	{
		error = errno;
		return false;
	}

	return true;
}

int Socket::Accept(int sockfd, std::string& client, int& error)
{
	struct sockaddr_un addr;
	int len=sizeof(addr);
	int connfd = accept4(sockfd, (struct sockaddr*) &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (connfd < 0)
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

	return connfd;
}

int Socket::Connect(int& sockfd, const std::string& dest, int& error)
{
	struct sockaddr_un addr;
	memset(addr, 0, sizeof(addr));
	addr.sun_family=AF_UNIX;
	strncpy(addr.sun_path, dest.c_str(),sizeof(addr.sun_path)-1);
	
	int connfd = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
	if(connfd < 0)
	{
		error = errno;
		Close(sockfd);
	}

	return connfd;
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


}

