#include <sstream>  
#include "ThreadPool.h"
#include "Exception.h"
#include "Macro.h"

namespace Stone{

std::string GetThreadName()
{
    std::stringstream sstream;
    std::string result;
    sstream << std::this_thread::get_id();
    sstream >> result;

    return std::move(result);
}

ThreadPool::ThreadPool(const std::string& name)
    :Name_(name),
    Running_(false)
{
    start(std::thread::hardware_concurrency()-1);
}

ThreadPool::~ThreadPool()
{
    if (Running_)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    Running_ = true;

    Threads_.clear();
    Threads_.reserve(numThreads);

    try
    {
        for (int i = 0; i < numThreads; ++i)
        {
            std::stringstream sstream;
            std::string id;
            sstream << i+1;
            sstream >> id;
            Threads_.push_back(std::thread(&ThreadPool::run, this, Name_+id));
        }
    }
    catch(...)
    {
        Running_ = false;
        throwSystemError("ThreadPool: thread creation failed");
    }

    if (numThreads == 0 && ThreadInitCallback_)
    {
        ThreadInitCallback_();
    }
}

void ThreadPool::stop()
{
    {
        std::lock_guard<std::mutex> lk(MutexHasData_);
        Running_ = false;
        HasData_.notify_all();
    }

    for(unsigned int i = 0;  i < Threads_.size(); i++)
    {
        if(Threads_[i].joinable())
        {
            Threads_[i].join();
            std::stringstream sstream;
            std::string id;
            sstream << Name_ << i+1;
            sstream >> id;
            _PRI("Thread stoped:%s", id.c_str());
        }
    }
}

FunctionWrapper ThreadPool::take(const std::string& thread)
{
    FunctionWrapper task;

#ifdef ESSIE_SPORT_MODE
    while((!Tasks_.BlockingGet(task)) && Running_)
    {
        std::this_thread::yield();
    }
#else
    if(!Tasks_.BlockingGet(task))
    {
        std::unique_lock<std::mutex> lk(MutexHasData_);
        HasData_.wait(lk, [&]{return (!Running_) || Tasks_.BlockingGet(task);});
    }

    std::lock_guard<std::mutex> lk(MutexHasFreeSpace_);
    HasFreeSpace_.notify_one();
#endif

    return std::move(task);
}

void ThreadPool::run(std::string thread)
{
    _PRI("Thread run:%s", thread.c_str());
    try
    {
        if (ThreadInitCallback_)
        {
            ThreadInitCallback_();
        }

        while (Running_)
        {
            FunctionWrapper task(take(thread));
            task();
        }
    }
    catch (...)
    {
        std::string msg =  "Unknown exception caught in ThreadPool's Thread : ";
        msg.append(thread);
        throwSystemError(msg.c_str());
    }
}


}


