#ifndef _STONE_SOCKETS_H_
#define _STONE_SOCKETS_H_
#include <string>

namespace Stone{

namespace Socket{

int Create(int& error);
bool Bind(int& sockfd, unsigned short port, int& error);
bool Listen(int sockfd, int& error);
int Accept(int sockfd, struct sockaddr_in& addr, int& error);
int Connect(int& sockfd, const std::string& dest, int& error);
void Close(int& sockfd);

struct sockaddr_in GetLocalAddr(int sockfd);
}

}


#endif

