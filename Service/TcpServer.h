#ifndef _STONE_TCP_SERVER_H_
#define _STONE_TCP_SERVER_H_
#include <vector>
#include <map>
#include "TcpConnection.h"
#include "LoopThread.h"
#include "AsyncQueue.h"
#include "TcpChannel.h"
#include "Noncopyable.h"

namespace Stone {

class TcpServer : private Noncopyable,
	public std::enable_shared_from_this<TcpServer>
{
public:
	TcpServer();
	TcpServer(unsigned short port);
	~TcpServer();
	bool Start();
	void Stop();

private:
	void newConnection(int sockfd, InetAddress& addr);
	void connectionInQueue(int index, int cond);
	
	unsigned short Port_;
	std::unique_ptr<LoopThread> ListenLoop_;
	std::vector<std::unique_ptr<LoopThread>> ConnectionLoops_;
	std::vector<std::shared_ptr<IO>> ConnectionIOs_;
	TcpChannel Channel_;
	std::atomic<bool> Started_;
	AsyncQueue<std::function<void()>> Queue_;
	int NoticeFd_;
	std::map<uint32_t, TcpConnectionPtr> Connections_;
	uint32_t ConnCount_;
};

}

#endif

