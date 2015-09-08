#ifndef _STONE_LOOP_THREAD_H_
#define _STONE_LOOP_THREAD_H_

#include <string>
#include <thread>
#include <future> 
#include <functional>
#include <memory>
#include <mutex>
#include "EventLoop.h"

namespace Stone{

class LoopThread : private Noncopyable
{
public:
	typedef std::function<bool(void)> InitCallback;

	LoopThread(std::string threadName, const InitCallback& cb = InitCallback());
	~LoopThread();
	bool Create();
	void Destroy();
	bool AddLoopIO(std::shared_ptr<IO>& io){return Loop_->AddIO(io);}
	void RemoveLoopIO(std::weak_ptr<IO>& wio){Loop_->RemoveIO(wio);}

 private:
	void threadProcess();

	std::string Name_;
	std::thread::id Creator_;
	std::unique_ptr<EventLoop> Loop_;
	std::unique_ptr<std::thread> Thread_;
	std::promise<bool> Created_;
	InitCallback Callback_;
	std::mutex Mutex_;
};

}


#endif

