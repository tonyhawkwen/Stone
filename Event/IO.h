#ifndef _STONE_IO_H_
#define _STONE_IO_H_
#include <functional>
#include <time.h>
#include <event2/event.h>

namespace Stone{

class IO
{
public:
	typedef std::function<void(int)> EventCallback;

	IO() : Fd_(-1), Condition_(0), Index_(-1), Timeval_(nullptr) {}
	IO(int fd, int cond) : Fd_(fd), Condition_(cond), Index_(-1), Timeval_(nullptr) {}
	IO(int fd, int cond, struct timeval* tv) : Fd_(fd), Condition_(cond), Index_(-1), Timeval_(tv) {}
	virtual ~IO(){}

	int Fd(){return Fd_;}
	int Condition(){return Condition_;}
	int Index(){return Index_;}
	struct timeval* Timeval(){return Timeval_;}
	void SetFd(int fd){Fd_ = fd;}
	void SetIndex(int index){Index_ = index;}
	void SetTimeval(struct timeval* tv){
		Timeval_ = tv;
	}
	void HandleEvent(int cond){if(Callback_)Callback_(cond);}
	void SetCallback(EventCallback&& cb){ Callback_ = std::move(cb); }

private:
	int Fd_;
	int Condition_;
	int Index_;
	struct timeval* Timeval_;
	EventCallback Callback_;

};

}

#endif

