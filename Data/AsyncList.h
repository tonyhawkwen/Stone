#ifndef _STONE_ASYNC_LIST_H_
#define _STONE_ASYNC_LIST_H_

#include "Noncopyable.h"
#include "Macro.h"
#include <atomic>
#include <memory>
#include <string.h>


namespace Stone{

template<class T>
struct Node ;

template<class T>
struct NodeMarkT{
	typedef Node<T>* NodePtr;

	NodeMarkT() noexcept : Next_(nullptr), /*LinkNum_(0),*/ Tag_(0), Deleted_(0){}
	explicit NodeMarkT(NodePtr next, /*unsigned int num,*/ unsigned int tag, unsigned int  deleted) noexcept :  
		Next_(next), /*LinkNum_(num),*/Tag_(tag), Deleted_(0){}

	NodePtr Next_;
	//unsigned int LinkNum_;
	unsigned int Tag_;
	unsigned int Deleted_;
};

template<class T>
struct Node {
	typedef NodeMarkT<T> NodeMark;

	T Value_;
	std::atomic<NodeMark> ALIGN_TO_AVOID_FALSE_SHARING Sign_; 
	//std::atomic<bool> ALIGN_TO_AVOID_FALSE_SHARING CanLink_;

	char Padding_[SIZE_OF_AVOID_FALSE_SHARING - sizeof(std::atomic<NodeMark>)];
};

template<class T>
class AsyncHashList : private Noncopyable
{
	typedef Node<T>* NodePtr;
	typedef NodeMarkT<T> NodeMark;

private:
	//increase a link number to prevent current element be deleted
	NodeMark increase(NodePtr node)
	{
		/*NodeMark cMark = node->Sign_.load(std::memory_order_acquire);
		NodeMark nMark;
		memcpy((void*)&nMark, (void*)&cMark, sizeof(NodeMark));
		++nMark.LinkNum_;
	
		while(!node->Sign_.compare_exchange_weak(cMark, nMark, std::memory_order_release))
		{
			memcpy((void*)&nMark, (void*)&cMark, sizeof(NodeMark));
			++nMark.LinkNum_;
		}*/
		NodeMark nMark = node->Sign_.load(std::memory_order_acquire);
		return std::move(nMark);
	}

	void decrease(NodePtr node)
	{
		/*NodeMark cMark = node->Sign_.load(std::memory_order_acquire);
		if(cMark.LinkNum_ == 0)
		{
			return;
		}

		NodeMark nMark;
		memcpy((void*)&nMark, (void*)&cMark, sizeof(NodeMark));
		--nMark.LinkNum_;
	
		while(!node->Sign_.compare_exchange_weak(cMark, nMark, std::memory_order_release))
		{
			memcpy((void*)&nMark, (void*)&cMark, sizeof(NodeMark));
			--nMark.LinkNum_;
		}*/

	}

	bool getPosition(const T& key, NodePtr& prev, NodePtr& cur, unsigned int& pLink, unsigned int& pTag, NodePtr& next, unsigned int& cLink, unsigned int& cTag)
	{
		prev = cur = next = nullptr;
		do
		{
			//now @cur pointer is going to be changed, the previous @prev pointer that point to @cur need to be decrease
			NodePtr prePre = prev;
			if(prePre)decrease(prePre);
			prev = &Head_;

			//increase current @prev pointer as @cur is going to point to @prev's Next_ to prevent being deleted
			NodePtr preCur = cur;
			//if(!prev->CanLink_.load(std::memory_order_relaxed))
			//{
			//	continue;
			//}
			NodeMark snapshot = increase(prev);
			cur = snapshot.Next_;
			pLink = 0;//snapshot.LinkNum_;
			pTag = snapshot.Tag_;

			do
			{
				if(nullptr == cur)
				{
					next = nullptr;
					return false;
				}
				
				//increase current @cur pointer as @next is going to point to @cur's Next_ to prevent being deleted
				//if(!cur->CanLink_.load(std::memory_order_relaxed))
				//{
				//	break;
				//}

				//now @next pointer is going to be changed, the previous @cur pointer that point to @next need to be decrease
				if(preCur)decrease(preCur);

				snapshot = increase(cur);
				bool cDeleted = snapshot.Deleted_;
				next = snapshot.Next_;
				cLink = 0;// = snapshot.LinkNum_;
				cTag = snapshot.Tag_;

				if(!cDeleted)
				{
					if(key < cur->Value_)
					{
						//now @prev moves to @cur, decrease @prev
						if(prev)decrease(prev);
							prev = cur;
					}
					else
					{
						return key == cur->Value_;
					}
				}
				else
				{
					//@cur will be removed when @prev's link number reduce to 1
					//NodeMark cMark;
					//while(true)
					//{
					//	cMark = prev->Sign_.load(std::memory_order_acquire);
					//	if(cMark.LinkNum_ <= 1)
					//	{
					//		break;
					//	}
					//}
					NodeMark cMark = prev->Sign_.load(std::memory_order_acquire);
					if(prev->Sign_.compare_exchange_strong(cMark, NodeMark(next, /*cLink,*/ pTag+1, 0), std::memory_order_acq_rel))
					{
						delete cur;
						cTag = pTag + 1;
					}
					else
					{
						break;
					}
				}

				/*Now @prev pointer's Next_ will have two pointer (@cur and @next) which point to,
				   currently @prev's LinkNum is still 1 which should be 2 and preCur should be set to @cur.
				   but for efficiency, set preCur to null to prevent the an unneccesary increase/decrease pair.*/
				preCur = nullptr;
				cur = next;
				pTag = cTag;
			}
			while(true);
		}
		while(true);

		return false;
	}

public:
	AsyncHashList()
	{
		//Head_.CanLink_.store(true, std::memory_order_relaxed);
	}

	bool Insert(const T& value)
	{
		NodePtr prev = nullptr;
		NodePtr cur = nullptr;
		NodePtr next = nullptr;
		unsigned int pTag = 0;
		unsigned int cTag = 0;
		unsigned int pLink = 0;
		unsigned int cLink = 0;

		NodePtr node = new Node<T>;
		node->Value_ = value;
		//node->CanLink_.store(true, std::memory_order_relaxed);

		while(true)
		{
			//reset @prev and @cur's link num first
			if(prev)decrease(prev);
			if(cur)decrease(cur);

			if(getPosition(value, prev, cur, pLink, pTag, next, cLink, cTag))
			{
				delete node;
				return false;
			}

			return true;
			while(true)
			{
				//move current @prev's link num - 1 to new node as @prev is going to be unlinked when method returns
				node->Sign_.store(NodeMark(cur, /*pLink == 0 ? 0 : pLink - 1,*/ 0, 0), std::memory_order_release);
				NodeMark cMark(cur, /*pLink,*/ pTag, 0);
				if(prev->Sign_.compare_exchange_strong(cMark, NodeMark(node, /*0,*/ pTag+1, 0), std::memory_order_acq_rel))
				{
					if(cur)decrease(cur);
					return true;
				}
				//if just link num is changed, use latest link num and try again
				//else if(cMark.Next_ == cur && cMark.Tag_ == pTag && cMark.Deleted_ == false)
				//{
				//	pLink = cMark.LinkNum_;
				//}
				else
				{
					break;
				}
			}
		}
	}

	bool Find(T& value)
	{
		NodePtr prev = nullptr;
		NodePtr cur = nullptr;
		NodePtr next = nullptr;
		unsigned int pTag = 0;
		unsigned int cTag = 0;
		unsigned int pLink = 0;
		unsigned int cLink = 0;


		bool ret = getPosition(value, prev, cur, pLink, pTag, next, cLink, cTag);
		if(ret)
		{
			value = cur->Value_;
		}

		if(prev)decrease(prev);
		if(cur)decrease(cur);

		return ret;
	}

	bool Find(const T& value)
	{
		T val = value;
		return Find(val);
	}

	bool Delete(const T& value)
	{
		NodePtr prev = nullptr;
		NodePtr cur = nullptr;
		NodePtr next = nullptr;
		unsigned int pTag = 0;
		unsigned int cTag = 0;
		unsigned int pLink = 0;
		unsigned int cLink = 0;

		while(true)
		{
			//reset @prev and @cur's link num first
			if(prev)decrease(prev);
			if(cur)decrease(cur);
			if(!getPosition(value, prev, cur, pLink, pTag, next, cLink, cTag))
			{
				return false;
			}

			NodeMark cMark(next, /*cLink,*/ cTag, 0);
			if(!cur->Sign_.compare_exchange_strong(cMark, NodeMark(next, /*cLink,*/ cTag+1, 1), std::memory_order_acq_rel))
			{
				continue;
			}

			//prev->CanLink_.store(false, std::memory_order_relaxed);
			//@cur will be removed when @prev's link number reduce to 1
			//while(true)
			//{
			//	cMark = prev->Sign_.load(std::memory_order_acquire);
			//	if(cMark.LinkNum_ <= 1)
			//	{
			//		break;
			//	}
			//}

			if(prev->Sign_.compare_exchange_strong(cMark, NodeMark(next, /*cLink,*/ pTag+1, 0), std::memory_order_acq_rel))
			{
				//delete cur;
				//prev->CanLink_.store(true, std::memory_order_relaxed);
			}
			else
			{
				if(prev)decrease(prev);
				if(cur)decrease(cur);
				getPosition(value, prev, cur, pLink, pTag, next, cLink, cTag);//check more here 
			}

			return true;
		}
	}

private:
	Node<T> Head_;
};

}

#endif

