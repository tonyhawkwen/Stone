#ifndef _STONE_IO_H_
#define _STONE_IO_H_
#include <functional>

namespace Stone{

class IO
{
public:
	typedef std::function<void(int)> EventCallback;

	IO() : Fd_(-1), Condition_(0), Index_(-1) {}
	IO(int fd, int cond) : Fd_(fd), Condition_(cond), Index_(-1) {}
	virtual ~IO(){}

	int Fd(){return Fd_;}
	int Condition(){return Condition_;}
	int Index(){return Index_;}
	void SetFd(int fd){Fd_ = fd;}
	void SetIndex(int index){Index_ = index;}
	void HandleEvent(int cond){if(Callback_)Callback_(cond);}
	void SetCallback(EventCallback&& cb){ Callback_ = std::move(cb); }

private:
	int Fd_;
	int Condition_;
	int Index_;
	EventCallback Callback_;

};

}

#endif

