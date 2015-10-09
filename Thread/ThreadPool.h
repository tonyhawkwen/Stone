#ifndef _STONE_THREAD_POOL_H_
#define _STONE_THREAD_POOL_H_

#include <thread>
#include <mutex>
#include <condition_variable> 
#include <functional>
#include <string>
#include <future>
#include <vector>
#include "Noncopyable.h"
#include "AsyncQueue.h"

namespace Stone{

class FunctionWrapper
{
public:
	FunctionWrapper() = default;

	template<typename FunctionType>
	FunctionWrapper(FunctionType&& f) noexcept
		:impl(new Impl<FunctionType>(std::move(f)))
	{
	}

	FunctionWrapper(FunctionWrapper&& other) noexcept
		:impl(std::move(other.impl))
	{
	}

	void operator()() 
	{
		if(impl)
		{
			return impl->call();
		}
	}

	FunctionWrapper& operator=(FunctionWrapper&& other)
	{
		impl=std::move(other.impl);
		return *this;
	}

	FunctionWrapper(const FunctionWrapper&)=delete;
	FunctionWrapper(FunctionWrapper&)=delete;
	FunctionWrapper& operator=(const FunctionWrapper&)=delete;

private:
	struct Base
	{
		virtual void call()=0;
		virtual ~Base() {}
	};

	template<typename FunctionType>
	struct Impl: Base
	{
		FunctionType F_;
		Impl(FunctionType&& f): F_(std::move(f)) {}
		void call() { return F_(); }
	};

	std::unique_ptr<Base> impl;
};

class ThreadPool : private Noncopyable
{
public:
	explicit ThreadPool(const std::string& name = "ThreadPool");
	~ThreadPool();
	void SetThreadInitCallback(const std::function<void ()>& cb){ ThreadInitCallback_ = cb; }

	template<typename FunctionType>
	std::future<typename std::result_of<FunctionType()>::type>
	Submit(FunctionType f)
	{
		typedef typename std::result_of<FunctionType()>::type ResultType;
		std::packaged_task<ResultType()> task(std::move(f));
		std::future<ResultType> res(task.get_future());

#ifdef ESSIE_SPORT_MODE
		while(!Tasks_.SinglePut(std::move(task)))
		{
			std::this_thread::yield();
		}
#else
		if(!Tasks_.SinglePut(std::move(task)))
		{
			std::unique_lock<std::mutex> lk(MutexHasFreeSpace_);
			HasFreeSpace_.wait(lk, [&]{return Tasks_.SinglePut(std::move(task));});
		}

		std::lock_guard<std::mutex> lk(MutexHasData_);
		HasData_.notify_one();
#endif

		return res;
	}

private:
	void start(int numThreads); // std::thread::hardware_concurrency() is suggested
	void stop();
	void run(std::string thread);
	FunctionWrapper take(const std::string& thread);

private:
	std::string Name_;
	std::vector<std::thread> Threads_;
	std::function<void()> ThreadInitCallback_;
	bool Running_;

	std::mutex MutexHasData_;
	std::mutex MutexHasFreeSpace_;
	std::condition_variable HasData_;
	std::condition_variable HasFreeSpace_;

	AsyncQueue<FunctionWrapper> Tasks_;
};


}

#endif

