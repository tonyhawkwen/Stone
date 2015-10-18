#include "TcpConnection.h"
#include "Macro.h"

namespace Stone {

TcpConnection::TcpConnection(
		const std::string& name,
		int sockfd,
		const InetAddress& localAddr, 
		const InetAddress& peerAddr):
	Name_(name),
	TcpIO_(new IO(sockfd, EV_READ | EV_PERSIST)),
	LocalAddr_(localAddr),
	PeerAddr_(peerAddr){
	}

TcpConnection::TcpConnection(
		std::string&& name,
		int sockfd, 
		InetAddress&& localAddr, 
		InetAddress&& peerAddr):
	Name_(std::move(name)),
	TcpIO_(new IO(sockfd, EV_READ | EV_PERSIST)),
	LocalAddr_(std::move(localAddr)),
	PeerAddr_(std::move(peerAddr)){
	}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::OnNewConnection(void)
{
	_DBG("New connection Fd %d From IP:%s, Port:%u", TcpIO_->Fd(), PeerAddr_.saddr.c_str(), PeerAddr_.port);

}


}

