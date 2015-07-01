#ifndef _ESSIE_EVENT_LOOP_H_
#define _ESSIE_EVENT_LOOP_H_

#include "Noncopyable.h"

namespace Essie{

class IO;

class EventLoop : private Noncopyable
{
public:
	EventLoop() = default;
	~EventLoop(){}

	virtual bool Prepare() = 0;
	virtual bool AddIO(std::shared_ptr<IO>& io) = 0;
	virtual void RemoveIO(std::weak_ptr<IO>& io) = 0;
	virtual void Loop() = 0;
	virtual void Quit() = 0;
};

};

#endif

