#include <sys/eventfd.h>
#include <arpa/inet.h>
#include "TcpServer.h"
#include "Sockets.h"
#include "EventLoop_LibEvent.h"

namespace Stone {

TcpServer::TcpServer():
	Port_(1080),
	ListenLoop_(new LoopThread("TcpListenThread")),
	Channel_(1080),
	Started_(false),
	Queue_(1024),
	NoticeFd_(-1),
	ConnCount_(0)
{
	ConnectionLoops_.reserve(std::thread::hardware_concurrency() - 1);
	ConnectionIOs_.reserve(std::thread::hardware_concurrency() - 1);
}

TcpServer::TcpServer(unsigned short port):
	Port_(port),
	ListenLoop_(new LoopThread("TcpListenThread")),
	Channel_(port),
	Started_(false),
	Queue_(1024),
	NoticeFd_(-1),
	ConnCount_(0)
{
	ConnectionLoops_.reserve(std::thread::hardware_concurrency() - 1);
	ConnectionIOs_.reserve(std::thread::hardware_concurrency() - 1);
}

TcpServer::~TcpServer()
{
	if(Started_.load(std::memory_order_acquire))
	{
		Stop();
	}
}

bool TcpServer::Start()
{
	_PRI("Start server...");
	bool started = Started_.load(std::memory_order_acquire);
	if(started)
	{
		_WRN("Server has been started!");
		return true;
	}

	NoticeFd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if(NoticeFd_ < 0)
	{
		_ERR("Get eventfd fail!");
		return false;
	}

	for(int i = 1; i < std::thread::hardware_concurrency(); ++i)
	{
		std::unique_ptr<EventLoop> loop(new EventLoopL());
		std::shared_ptr<IO> io(new IO(NoticeFd_, EV_READ | EV_PERSIST));
		io->SetCallback(std::bind(&TcpServer::connectionInQueue, shared_from_this(), i - 1, std::placeholders::_1));
		loop->AddIO(io);
		ConnectionIOs_.push_back(io);
	
		char threadName[32];
		sprintf(threadName, "TcpThread%d", i);
		std::unique_ptr<LoopThread> thread(new LoopThread(threadName));
		thread->Create(std::move(loop));
		ConnectionLoops_.push_back(std::move(thread));
	}

	std::unique_ptr<EventLoop> loop(new EventLoopL());
	if(!Channel_.Create())
	{
		_ERR("create tcp channel fail!");
		return false;
	}

	Channel_.SetReadCallback(std::bind(&TcpServer::newConnection,
					shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	loop->AddIO(Channel_.TcpIO());
	if(!Channel_.Listen())
	{
		return false;
	}
	ListenLoop_->Create(std::move(loop));

	Started_.store(true, std::memory_order_release);
	return true;
}

void TcpServer::Stop()
{
	ListenLoop_->Destroy();
	ConnectionLoops_.clear();
	ConnectionIOs_.clear();
	//Queue_.clear();
	close(NoticeFd_);
	Started_.store(false, std::memory_order_release);
}

void TcpServer::newConnection(int sockfd, InetAddress& addr)
{
	_DBG("New connection Fd %d From IP:%s, Port:%u", sockfd, addr.saddr.c_str(), addr.port);
	
	char buf[32];
	snprintf(buf, sizeof buf, ":%u_%u", Port_, ++ConnCount_);
	std::string connName = buf;
	
	struct sockaddr_in local = Socket::GetLocalAddr(sockfd);
	InetAddress localAddr;
	localAddr.addr = local.sin_addr.s_addr;
	localAddr.saddr = inet_ntoa(local.sin_addr);
	localAddr.port = local.sin_port;

	TcpConnectionPtr conn(new TcpConnection(
				std::move(connName),
				sockfd,
				std::move(localAddr),
				std::move(addr)
				));

	while(!Queue_.SinglePut(std::bind(
					&TcpConnection::OnNewConnection, 
					conn)))
	{
		usleep(100);
	}

	Connections_[ConnCount_] = std::move(conn);
	eventfd_write(NoticeFd_, 1);
}

void TcpServer::connectionInQueue(int index, int cond)
{
	_DBG("receive connection, index : %d cond : %d", index, cond);
	auto& thread = ConnectionLoops_[index];
	eventfd_t num = 0;
	eventfd_read(NoticeFd_, &num);
	for(int i = 0; i < num; ++i)
	{
		std::function<void()> fn;
		if(!Queue_.BlockingGet(fn))
		{
			break;
		}
		
		_DBG("get one request success!");
		fn();
	}
}

}
