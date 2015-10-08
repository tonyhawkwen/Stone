#include <iostream>
#include <signal.h>
#include <event.h>
#include "EventLoop_LibEvent.h"
#include "LoopThread.h"
#include "TcpChannel.h"
#include "Macro.h"
#include <string.h>
using namespace Stone;

#define MEM_SIZE 1024

//temp code, to be moved to specific class
TcpChannel* channel = nullptr;
LoopThread* server = nullptr;
std::shared_ptr<IO> readIO;


bool gExitServer = false;

void SignalQuit(int signo)
{
    //here write need catch signal
    _PRI("signal %d caught, search server will quit", signo);
    gExitServer = true;
}

void SingalHandle()
{
    signal(SIGPIPE, SIG_IGN);//ignore signal of SIGPIPE
    signal(SIGINT, SignalQuit);
    signal(SIGTERM, SignalQuit);
}

void DoRead(int cond)
{
	_PRI("Do Read...");
	char buff[4096] = {0,};
	int size = recv(readIO->Fd(), buff, 4095, 0);
	buff[size] = '\0';
	_PRI("Get Data %s", buff);
	if(size <= 0)
	{
		std::weak_ptr<IO> wio(readIO);
		server->RemoveLoopIO(wio);
		close(readIO->Fd());
	}
}

void OnRead(int sockfd, const InetAddress& addr)
{
	_PRI("Fd %d From IP:%s, Port:%u", sockfd, addr.saddr.c_str(), addr.port);
	readIO.reset(new IO(sockfd, EV_READ | EV_PERSIST));
	readIO->SetCallback(std::bind(DoRead, std::placeholders::_1));
	server->AddLoopIO(readIO);
}

int main(void)
{
    _PRI("Begin server...");
	SingalHandle();
	
	std::unique_ptr<EventLoop> loop(new EventLoopL());
	
	_PRI("Add socket IO begin...:");
	channel = new TcpChannel;
    if(!channel->Create())
	{
		_ERR("create tcp channel fail!");
		return -1;
	}

	channel->SetReadCallback(OnRead);
	loop->AddIO(channel->TcpIO());
	channel->Listen();

	//after this line, you can not use loop variable anymore
	server = new LoopThread("SocketServer");
    server->Create(loop);

	while(!gExitServer)
	{
		sleep(1);
	}

	delete server;
	delete channel;
	_PRI("End server!");      
    return 0;
}

