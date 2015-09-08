#include <iostream>
#include <signal.h>
#include <event.h>
#include "LoopThread.h"
#include "TcpChannel.h"
#include "Macro.h"

using namespace Stone;

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

bool AddSocketIO()
{
    _PRI("Add socket IO begin...:");

	TcpChannel* channel = new TcpChannel;
	if(!channel->Create())
	{
		_ERR("create tcp channel fail!");
		return false;
	}
	channel->SetReadCallback(OnRead);
	server->AddLoopIO(channel->TcpIO());
	channel->Listen();

    return true;
}

int main(void)
{
    _PRI("Begin server...");
	SingalHandle();
    server = new LoopThread("SocketServer", AddSocketIO);
    server->Create();

	while(!gExitServer)
	{
		sleep(1);
	}

	delete channel;
	delete server;
	_PRI("End server!");      
    return 0;
}
