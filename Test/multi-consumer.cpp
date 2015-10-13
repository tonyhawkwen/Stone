#include <iostream>
#include <signal.h>
#include <event.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <gflags/gflags.h>
#include "EventLoop_LibEvent.h"
#include "IO.h"
#include "LoopThread.h"
#include "Macro.h"
#include "Timer.h"
using namespace Stone;

LoopThread* server = nullptr;
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

Timer* timer;
bool Timeout()
{
	_PRI("Timeout...");
	return true;
}
int evtfd;
void DoRead1(int cond)
{
	_PRI("Do Read1...");
	eventfd_t quit = 1;
	eventfd_read(evtfd, &quit);
	_PRI("1****%llu*****", quit);
	timer = new Timer(Timeout, 1000, 3);
	timer->Run();
}

void DoRead2(int cond)
{
	_PRI("Do Read2...");
	sleep(3);
	//eventfd_t quit = 1;
	//eventfd_read(evtfd, &quit);
	//_PRI("2****%llu*****", quit);
}

int main(int argc, char** argv)
{
    _PRI("Begin server...");
	SingalHandle();
	google::ParseCommandLineFlags(&argc, &argv, true);

	std::unique_ptr<EventLoop> loop1(new EventLoopL());
	std::unique_ptr<EventLoop> loop2(new EventLoopL());

	_PRI("Add socket IO begin...:");
	evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	std::shared_ptr<IO> myIO1, myIO2;
	myIO1.reset(new IO(evtfd, EV_READ | EV_PERSIST));
	myIO1->SetCallback(DoRead1);

	myIO2.reset(new IO(evtfd, EV_READ | EV_PERSIST));
	myIO2->SetCallback(DoRead2);

	loop1->AddIO(myIO1);
	loop2->AddIO(myIO2);

	//after this line, you can not use loop variable anymore
	LoopThread* server1 = new LoopThread("Server1");
	server1->Create(std::move(loop1));
	
	LoopThread* server2 = new LoopThread("Server2");
    server2->Create(std::move(loop2));
	
	uint64_t count = 4;
	write(evtfd, &count, sizeof(count));
	while(!gExitServer)
	{
		sleep(1);
		//eventfd_t quit = 10;
		//eventfd_write(evtfd, quit);
	}

	delete server1;
	delete server2;
	_PRI("End server!");      
    return 0;
}

