#ifndef _ESSIE_EVENT_LOOP_LIBEVENT_H_
#define _ESSIE_EVENT_LOOP_LIBEVENT_H_

#include <thread>
#include <queue>
#include <functional>
#include "EventLoop.h"

struct event_config;
struct event_base;

#define MAX_EVENTS_SIZE 256

namespace Essie{

struct LoopData;

class EventLoopL : public EventLoop
{
public:
	EventLoopL();

	virtual ~EventLoopL();
	bool Prepare();
	bool AddIO(std::shared_ptr<IO>& io);
	void RemoveIO(std::weak_ptr<IO>& io);
	void Loop();
	void Quit();
	void EventLoopCallBack(int index, short cond);

private:
	void quitAsync(int);

	std::thread::id Creator_;
	struct event_config* EventConfig_;
	struct event_base* EventBase_;
	std::unique_ptr<LoopData> Datas_[MAX_EVENTS_SIZE];
	int CurIndex_;
	std::queue<int> IdleIndexs_;
	std::shared_ptr<IO> Wakeup_;
};

}

#endif


