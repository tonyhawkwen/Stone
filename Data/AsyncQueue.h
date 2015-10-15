#ifndef _STONE_ASYNCQUEUE_H_
#define _STONE_ASYNCQUEUE_H_

#include "Macro.h"
#include "Noncopyable.h"
#include <atomic>
#include <cassert>
#include <type_traits>
#include <cstdlib>
#include <iostream>

namespace Stone{

template<class T>
struct SingleElement
{
	T Content_;
#ifdef ESSIE_SPORT_MODE
	//here use the simple way to avoid false sharing if need, which may waste a lot of memory
	char Padding_[SIZE_OF_AVOID_FALSE_SHARING - (sizeof(T) % SIZE_OF_AVOID_FALSE_SHARING)]; //fill out the last cache line at the end of struct
#endif
};

template<class T>
class AsyncQueue : private Noncopyable
{

//static_assert(std::is_nothrow_constructible<T,T&&>::value, "T must have a noexcept move constructor");

public:
	AsyncQueue()
		: Size_(DEFAULT_QUEUE_BUFFER_SIZE)
		, RingBuffer_(static_cast<SingleElement<T>*>(std::malloc(sizeof(SingleElement<T>) * DEFAULT_QUEUE_BUFFER_SIZE)))
		, ReadIndex_(0)
		, WriteIndex_(0)
		,RCommitIndex_(0)
		, WCommitIndex_(0)
	{
	}

	explicit AsyncQueue(unsigned int size)
		:Size_(size)
		, RingBuffer_(static_cast<SingleElement<T>*>(std::malloc(sizeof(SingleElement<T>) * size)))
		, ReadIndex_(0)
		, WriteIndex_(0)
		,RCommitIndex_(0)
		, WCommitIndex_(0)
	{
		assert(size > 2);
	}

	AsyncQueue(AsyncQueue<T>&& rhs) noexcept 
		: Size_(rhs.Size_)
		, RingBuffer_(rhs.RingBuffer_) 
		, ReadIndex_(rhs.ReadIndex_.load(std::memory_order_relaxed)) 
		, WriteIndex_(rhs.WriteIndex_.load(std::memory_order_relaxed))
		,RCommitIndex_(rhs.RCommitIndex_.load(std::memory_order_relaxed))
		, WCommitIndex_(rhs.WCommitIndex_.load(std::memory_order_relaxed))
	{
		rhs.Size_ = 0;
		rhs.RingBuffer_ = NULL;
		rhs.ReadIndex_.store(0, std::memory_order_relaxed);
		rhs.WriteIndex_.store(0, std::memory_order_relaxed);
		rhs.RCommitIndex_.store(0, std::memory_order_relaxed);
		rhs.WCommitIndex_.store(0, std::memory_order_relaxed);
	}

	~AsyncQueue() 
	{
		//if (!std::has_trivial_destructor<T>::value)//for gnu 4.6
		if(!std::is_trivially_destructible<T>::value)//for gnu 4.9
		{
			int read = ReadIndex_;
			int end = WriteIndex_;
			while (read != end)
			{
				RingBuffer_[read].Content_.~T();
				if (++read == Size_)
				{
					read = 0;
				}
			}
		}

		std::free(RingBuffer_);
	}

	bool SingleGet(T& data)//this is for single consumer
	{
		auto curRead = ReadIndex_.load(std::memory_order_relaxed);
		
		if(curRead == WCommitIndex_.load(std::memory_order_acquire))
		{
			//queue is empty
			return false;
		}

		auto nextRead = curRead + 1;
		if (nextRead == Size_)
		{
			nextRead = 0;
		}

		ReadIndex_.store(nextRead, std::memory_order_relaxed);

		data = std::move(RingBuffer_[curRead].Content_);
		RingBuffer_[curRead].Content_.~T();

		RCommitIndex_.store(nextRead, std::memory_order_release);
		return true;
	}

	bool BlockingGet(T& data)//this is for multi consumer
	{
		auto curRead = ReadIndex_.load(std::memory_order_relaxed);
		auto nextRead = curRead;

		do
		{
			if(curRead == WCommitIndex_.load(std::memory_order_acquire))
			{
				//queue is empty
				return false;
			}

			nextRead = curRead + 1;
			if (nextRead == Size_)
			{
				nextRead = 0;
			}
		}while(!ReadIndex_.compare_exchange_weak(curRead, nextRead, std::memory_order_relaxed));

		data = std::move(RingBuffer_[curRead].Content_);
		RingBuffer_[curRead].Content_.~T();

		//when current commit index goes to current write, commit the write and let read index know
		auto tmpCommit = curRead;
		auto tmpNextCommit = curRead;

		do
		{
			tmpCommit = curRead;
			tmpNextCommit = curRead + 1;
			if(tmpNextCommit == Size_)
			{
				tmpNextCommit = 0;
			}
		}
		while(!RCommitIndex_.compare_exchange_weak(tmpCommit, tmpNextCommit, std::memory_order_acq_rel));

		return true;
	}

	template<class ...Args>
	bool SinglePut(Args&&... recordArgs)//this is for single producer
	{
		//get current write index
		auto curWrite = WriteIndex_.load(std::memory_order_relaxed);
		auto nextWrite = curWrite + 1;
		if(nextWrite == Size_)
		{
			nextWrite = 0;
		}

		//if queue is full(the last item will be kept empty to make sure commit index is always larger than read index), return false
		if(nextWrite == RCommitIndex_.load(std::memory_order_acquire))
		{
			return false;
		}

		WriteIndex_.store(nextWrite, std::memory_order_relaxed);

		//update current write index's data
		new (&RingBuffer_[curWrite].Content_) T(std::forward<Args>(recordArgs)...);

		//when current commit index goes to current write, commit the write and let read index know
		WCommitIndex_.store(nextWrite, std::memory_order_release);

		return true;
	}

	template<class ...Args>
	bool BlockingPut(Args&&... recordArgs)//this is for multi producer
	{
		//get current write index
		auto curWrite = WriteIndex_.load(std::memory_order_relaxed);
		auto nextWrite = curWrite;

		//try to get next write index, compare current write index, if it is same with before then update m_WriteIndex to next index.
		do
		{
			nextWrite = curWrite + 1;
			if(nextWrite == Size_)
			{
				nextWrite = 0;
			}

			//if queue is full(the last item will be kept empty to make sure commit index is always larger than read index), return false
			auto curRCommit = RCommitIndex_.load(std::memory_order_acquire);
			if(nextWrite == curRCommit)
			{
				return false;
			}
		}while(!WriteIndex_.compare_exchange_weak(curWrite, nextWrite, std::memory_order_relaxed));

		//update current write index's data
		new (&RingBuffer_[curWrite].Content_) T(std::forward<Args>(recordArgs)...);

		//when current commit index goes to current write, commit the write and let read index know
		auto tmpCommit = curWrite;
		auto tmpNextCommit = curWrite;
		int i = 0;
		do
		{
			i++;
			tmpCommit = curWrite;
			tmpNextCommit = curWrite + 1;
			if(tmpNextCommit == Size_)
			{
				tmpNextCommit = 0;
			}
		}
		while(!WCommitIndex_.compare_exchange_weak(tmpCommit, tmpNextCommit, std::memory_order_acq_rel));

		return true;
	}

	bool IsQueueEmpty() const 
	{
		auto curRead = ReadIndex_.load(std::memory_order_relaxed);
		if(curRead == WCommitIndex_.load(std::memory_order_acquire))
		{
			//queue is empty
			return true;
		}

		return false;
	}

	bool IsQueueFull(void) const
	{
		auto nextRecord = WriteIndex_.load(std::memory_order_relaxed) + 1;
		if (nextRecord == Size_)
		{
			nextRecord = 0;
		}

		if (nextRecord == RCommitIndex_.load(std::memory_order_acquire)) 
		{
			return true;
		}

		return false;
	}

private:
	unsigned int Size_; //not changed
	SingleElement<T>* const RingBuffer_;//not changed

	std::atomic<unsigned int> ALIGN_TO_AVOID_FALSE_SHARING ReadIndex_; //current available read index, always changed
	std::atomic<unsigned int> ALIGN_TO_AVOID_FALSE_SHARING WriteIndex_;//current available write index, always changed
	std::atomic<unsigned int> ALIGN_TO_AVOID_FALSE_SHARING RCommitIndex_; //current commit reading index, always changed
	std::atomic<unsigned int> ALIGN_TO_AVOID_FALSE_SHARING WCommitIndex_; //current commit writing index, always changed

	char Padding_[SIZE_OF_AVOID_FALSE_SHARING - sizeof(std::atomic<unsigned int>)]; //fill out the last cache line at the end of struct
};


}

#endif


