#ifndef _STONE_SOCKETS_H_
#define _STONE_SOCKETS_H_
#include <string>

namespace Stone{

namespace Socket{

int Create(int& error);
bool Bind(int& sockfd, const std::string& path, int& error);
bool Listen(int sockfd, int& error);
int Accept(int sockfd, std::string& client, int& error);
int Connect(int& sockfd, const std::string& dest, int& error);
void Close(int& sockfd);

}

}


#endif

