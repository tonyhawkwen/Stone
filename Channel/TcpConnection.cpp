#include <sys/uio.h>
#include "TcpConnection.h"
#include "LoopThread.h"
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
	if(TcpIO_)
	{
		TcpIO_->SetCallback(
					std::bind(&TcpConnection::handleRead, shared_from_this(), std::placeholders::_1));
		if(!LoopThread::AddLoopIOLocal(TcpIO_))
		{
			_ERR("Add IO to local loopthread fail!");
			return;
		}
	}
}

void TcpConnection::handleRead(int cond)
{
	_ERR("handle read %d", cond);
	char buf[1024];
	struct iovec vec[1];
	vec[0].iov_base = buf;
	vec[0].iov_len = sizeof(buf);

	const ssize_t n = readv(TcpIO_->Fd(), vec, 1);
	if(n == 0)
	{
		_WRN("Remote side of connection was closed.");
		if(CloseCallback_)
		{
			CloseCallback_(shared_from_this());
		}
		std::weak_ptr<IO> io(TcpIO_);
		LoopThread::RemoveLoopIOLocal(io);
		close(TcpIO_->Fd());
		return;
	}
	else if(n < 0)
	{
		//TODO: if set nonblocking
		//if (errno == EAGAIN)
		//{
		//    continue;
		//}
		
		//TODO: after adding buffer, fix this one
		//if(errno == EINTR)
		//{
		//	continue;
		//}
		_ERR("read error %d!", errno);
		return;
	}

	std::string buff(buf, n);
	_DBG("Get data : %s size: %d", buf, n);
	if(MsgCallback_)
	{
		MsgCallback_(shared_from_this(), buff);
	}

	//for test
	//write(TcpIO_->Fd(), "123", 3);
}

}

