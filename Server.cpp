#include <iostream>
#include <signal.h>
#include <event.h>
#include <gflags/gflags.h>
#include "TcpServer.h"
#include "Macro.h"
using namespace Stone;

DEFINE_int32(port, 1080, "What port to listen on");
#define MEM_SIZE 1024

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

int main(int argc, char** argv)
{
    _PRI("Begin server...");
	SingalHandle();
	google::ParseCommandLineFlags(&argc, &argv, true);

	_PRI("Start tcp server begin...:");
	std::shared_ptr<TcpServer> server(new TcpServer(FLAGS_port));
	if(!server->Start())
	{
		_ERR("Start fail!");
		return -1;
	}

	while(!gExitServer)
	{
		sleep(1);
	}

	server->Stop();
	_PRI("End server!");      
    return 0;
}

