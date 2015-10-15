#include <event2/event.h>
#include <sys/eventfd.h>
#include "TcpServer.h"
#include "EventLoop_LibEvent.h"

namespace Stone {

TcpServer::TcpServer():
	ListenLoop_(new LoopThread("TcpListenThread")),
	Channel_(1080),
	Started_(false),
	Queue_(1024),
	NoticeFd_(-1)
{
	ConnectionLoops_.reserve(std::thread::hardware_concurrency() - 1);
	ConnectionIOs_.reserve(std::thread::hardware_concurrency() - 1);
}

TcpServer::TcpServer(unsigned short port):
	ListenLoop_(new LoopThread("TcpListenThread")),
	Channel_(port),
	Started_(false),
	Queue_(1024),
	NoticeFd_(-1)
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
		io->SetCallback(std::bind(&TcpServer::connectionInQueue, this));
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

	Channel_.SetReadCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
	loop->AddIO(Channel_.TcpIO());
	Channel_.Listen();
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

void TcpServer::newConnection(int sockfd, const InetAddress& addr)
{

}

void TcpServer::connectionInQueue(void)
{

}
}
