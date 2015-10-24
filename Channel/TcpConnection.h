#ifndef _STONE_TCP_CONNECTION_H_
#define _STONE_TCP_CONNECTION_H_

#include <memory>
#include "Noncopyable.h"
#include "InetAddress.h"
#include "IO.h"

namespace Stone {

class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void (const TcpConnectionPtr&)> Callback;
typedef std::function<void (const TcpConnectionPtr&, std::string&)> MessageCallback;

class TcpConnection : Noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
	TcpConnection(const std::string& name,int sockfd,
			const InetAddress& localAddr, const InetAddress& peerAddr);
	TcpConnection(std::string&& name, int sockfd, 
			InetAddress&& localAddr, InetAddress&& peerAddr);

	~TcpConnection();
	std::shared_ptr<IO>& TcpIO(){
		return TcpIO_;
	}
	
	void OnNewConnection(void);

	void SetConnectionCallback(Callback&& cb)
	{
		ConnectionCallback_ = std::move(cb); 
	}

	void SetCloseCallback(Callback&& cb)
	{
		CloseCallback_ = std::move(cb);
	}

	void SetReadCallback(Callback&& cb)
	{
		ReadCallback_ = std::move(cb); 
	}

	void SetMsgCallback(MessageCallback&& cb)
	{
		MsgCallback_ = std::move(cb);
	}

private:
	void handleRead(int cond);

	std::shared_ptr<IO> TcpIO_;
	std::string Name_;
	InetAddress LocalAddr_;
	InetAddress PeerAddr_;
	Callback ConnectionCallback_;
	Callback ReadCallback_;
	Callback CloseCallback_;
	MessageCallback MsgCallback_;
};

}

#endif
