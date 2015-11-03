#include <sys/uio.h>
#include <string.h>
#include "TcpConnection.h"
#include "LoopThread.h"
#include "Macro.h"
#include "Protocal.h"

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
		closeConnection();
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
		closeConnection();
		return;
	}

	_DBG("Get data size: %d", n);
	for(ssize_t i = 0; i < n; ++i)
	{
		_DBG("Data[%d]:%x", i, buf[i]);
	}
	//TO DO: move to protocal class
	//if(MsgCallback_)
	//{
	//	MsgCallback_(shared_from_this(), buff);
	//}
	//Check Type
	Protocal request(buf, n);
	unsigned int device = request.GetDevice();
	unsigned int type = request.GetType();
	_DBG("device :%u, type:%u", device, type);
	char msg[1024];
	unsigned int length = 0;
	//1. heart beat
	if(type == 0x1)
	{
		_DBG("heart beat!");
		unsigned int data[1];
		data[0] = 0x0;
		Protocal response(0, 0x1, (char*)data, 4);
		length = response.GetBuffer(msg);
	}
	//2. auth
	else if(device == 0x1 && type == 0x2)
	{
		_DBG("App auth!");
		const char* rev = NULL;
		unsigned int size = request.GetData(rev);
		const char* p = rev;
		while(*p++);
		_DBG("User:%s, Password:%s", rev, p);
		
		unsigned int data[1];
		data[0] = 0x0;
		Protocal response(0, 0x2, (char*)data, 4);
		length = response.GetBuffer(msg);
	}
	//3. gateway info
	else if(device == 0x1 && type == 0x3)
	{
		_DBG("Request info!");
		const char* rev = NULL;
		unsigned int size = request.GetData(rev);
		std::string duid(rev, size);
		_DBG("DUID:%s", duid.c_str());

		unsigned int data[3];
		data[0] = 0x3;
		data[1] = 0x0;
		data[2] = 0x0;
		Protocal response(0, 0x2, (char*)data, 12);
		length = response.GetBuffer(msg);
	}
	_DBG("Response length:%u", length);
	for(ssize_t i = 0; i < length; ++i)
	{
		_DBG("Data[%d]:%x", i, msg[i]);
	}
	write(TcpIO_->Fd(), (void*)msg, length);
}

void TcpConnection::closeConnection()
{
	if(CloseCallback_)
	{
		CloseCallback_(shared_from_this());
	}
	LoopThread::RemoveLoopIOLocal(TcpIO_);
	close(TcpIO_->Fd());
}

}

