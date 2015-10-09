#include <sstream>
#include "Macro.h"
#include "LoopThread.h"
#include "Exception.h"
#include "EventLoop_LibEvent.h"

namespace Stone{

LoopThread::LoopThread(std::string threadName, const InitCallback& cb)
	:Name_(threadName),
	Callback_(cb)
{
}

LoopThread::~LoopThread()
{
	Destroy();
}

void LoopThread::Destroy()
{
	_DBG("LoopThread::Destroy()");
	/*if(std::this_thread::get_id() != Creator_)
	{
		_ERR("only creator thread can destroy this thread!");
		return;
	}*/

	std::lock_guard<std::mutex> lock(Mutex_);
	if (Loop_)
	{
		Loop_->Quit();

		if(Thread_->joinable())
		{
			Thread_->join();
		}

		Loop_.reset();
	}
}

bool LoopThread::Create(std::unique_ptr<EventLoop>&& loop)
{
	std::lock_guard<std::mutex> lock(Mutex_);
	Creator_ = std::this_thread::get_id();

	bool ret = false;
	try
	{
		Loop_ = std::move(loop);
		std::future<bool> fut = Created_.get_future();
		Thread_ .reset(new std::thread(&LoopThread::threadProcess, this));

		ret = fut.get();
	}
	catch(const std::exception& ex)
	{
		std::stringstream sstream;
		sstream << "LoopThread: thread creation failed : " << ex.what();
		throwSystemError(sstream.str().c_str());
	}
	catch(...)
	{
		throwSystemError("LoopThread: thread creation failed");
	}

	_DBG("Create %s", ret ? "success" : "fail");
	return ret;
}

void LoopThread::threadProcess()
{
	try 
	{
		bool ret = false;
		do
		{
			if(!Loop_)
			{
				Loop_.reset(new EventLoopL());
			}

			Loop_->SetOwner(std::this_thread::get_id());
			
			if (Callback_ && (!Callback_()))
			{
				break;
			}

			ret = Loop_->Prepare();
		}while(0);

		Created_.set_value(ret);
		if(!ret)
		{
			_ERR("Loop prepare fail!");
			return;
		}
	}
	catch(const std::exception& ex)
	{
		std::stringstream sstream;	
		std::string result;
		sstream << "LoopThread thread creation failed : " << ex.what();
		result = sstream.str();
		Created_.set_exception(std::make_exception_ptr(Exception(result)));
		return;
	}
	catch (...) 
	{
		Created_.set_exception(std::current_exception());
		return;
	}

	Loop_->Loop();

	_DBG("threadProcess end.");
}

}


