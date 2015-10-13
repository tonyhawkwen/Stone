#ifndef _STONE_TIMER_H_
#define _STONE_TIMER_H_

#include "LoopThread.h"
#include "Noncopyable.h"
#include "Macro.h"
#include "IO.h"

namespace Stone {

class Timer : private Noncopyable
{
public:
	Timer() : 
		TimerIO_(new IO(-1, 0)),
		Times_(-1),
		Counter_(0),
		Loop_(nullptr),
		Running_(false){
			Timeval_.tv_sec = 0;
			Timeval_.tv_usec = 0;
		}

	Timer(const std::function<bool()>& cb) :
		TimerIO_(new IO(-1, 0)),
		Times_(-1),
		Counter_(0),
		Loop_(nullptr),
		Callback_(std::move(cb)),
		Running_(false){
			Timeval_.tv_sec = 0;
			Timeval_.tv_usec = 0;
		}

	Timer(const std::function<bool()>& cb, uint32_t tv) :
		TimerIO_(new IO(-1, 0)),
		Times_(-1),
		Counter_(0),
		Loop_(nullptr),
		Callback_(std::move(cb)),
		Running_(false){
			Timeval_.tv_sec = tv / 1000;
			Timeval_.tv_usec = (tv % 1000) * 1000;
		}

	Timer(const std::function<bool()>& cb, uint32_t tv, int32_t times) :
		TimerIO_(new IO(-1, 0)),
		Times_(times),
		Counter_(0),
		Loop_(nullptr),
		Callback_(std::move(cb)),
		Running_(false){
			Timeval_.tv_sec = tv / 1000;
			Timeval_.tv_usec = (tv % 1000) * 1000;
		}

	~Timer()
	{
		std::weak_ptr<IO> wio(TimerIO_);
		Loop_->RemoveLoopIO(wio);
		Running_ = false;
		Counter_ = 0;
	}

	bool Run(LoopThread* loop)
	{
		if(loop == nullptr)
		{
			_ERR("No active loop in this thread!");
			return false;
		}
		
		Loop_ = loop;
		if(Running_)
		{
			return Reset();
		}
		
		if(Timeval_.tv_sec == 0 && Timeval_.tv_usec == 0)
		{
			_ERR("Timeval is wrong!");
			return false;
		}

		TimerIO_->SetTimeval(&Timeval_);
		TimerIO_->SetCallback(std::bind(&Timer::handle, this, std::placeholders::_1));
		bool ret = loop->AddLoopIO(TimerIO_);

		Running_ = true;
		return ret;
	}
	
	bool Run()
	{
		return Run(LoopThread::GetLocalLoopThread());
	}

	bool Reset(int32_t times)
	{
		if(Loop_ == nullptr)
		{
			_ERR("No active loop found!");
			return false;
		}
		Times_ = times;
		Counter_ = 0;

		if(times == 0)
		{
			return true;
		}

		bool ret = Loop_->AddLoopIO(TimerIO_);
		Running_ = true;
		return ret;
	}
	
	bool Reset()
	{
		return Reset(Times_);
	}

private:
	void handle(int cond)
	{
		bool ret = false;
		if(Times_ == -1 || Counter_++ < Times_)
		{
			if(Callback_)
			{
				ret = Callback_();
			}
		}

		if(ret)
		{
			Loop_->AddLoopIO(TimerIO_);
		}
	}

	std::shared_ptr<IO> TimerIO_;
	struct timeval Timeval_;
	int32_t Times_;
	int32_t Counter_;
	LoopThread* Loop_;
	std::function<bool()> Callback_;
	bool Running_;
};

}

#endif
