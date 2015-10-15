#ifndef _STONE_TCP_SERVER_H_
#define _STONE_TCP_SERVER_H_
#include <vector>
#include "LoopThread.h"
#include "AsyncQueue.h"
#include "TcpChannel.h"
#include "Noncopyable.h"

namespace Stone {

class TcpServer : private Noncopyable
{
public:
	TcpServer();
	TcpServer(unsigned short port);
	~TcpServer();
	bool Start();
	void Stop();

private:
	void newConnection(int sockfd, const InetAddress& addr);
	void connectionInQueue(void);

	std::unique_ptr<LoopThread> ListenLoop_;
	std::vector<std::unique_ptr<LoopThread>> ConnectionLoops_;
	std::vector<std::shared_ptr<IO>> ConnectionIOs_;
	TcpChannel Channel_;
	std::atomic<bool> Started_;
	AsyncQueue<std::function<void()>> Queue_;
	int NoticeFd_;
};

}

#endif

