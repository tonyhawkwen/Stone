#ifndef _STONE_EVENT_LOOP_LIBEVENT_H_
#define _STONE_EVENT_LOOP_LIBEVENT_H_

#include <thread>
#include <queue>
#include <functional>
#include "EventLoop.h"

struct event_config;
struct event_base;

#define MAX_EVENTS_SIZE 256

namespace Stone{

struct LoopData;

class EventLoopL : public EventLoop
{
public:
    EventLoopL();

    virtual ~EventLoopL();
    bool Prepare();
    bool AddIO(std::shared_ptr<IO>& io) override;
    void RemoveIO(std::shared_ptr<IO>& io) override;
    void FreezeIO(std::shared_ptr<IO>& io) override;
    bool RestartIO(std::shared_ptr<IO>& io) override;
    void SetOwner(std::thread::id owner) {
        Owner_ = owner;
    }
    void Loop();
    void Quit();
    void EventLoopCallBack(int index, short cond);

private:
    void quitAsync(int);
    void removeAllIO();

    std::thread::id Owner_;
    struct event_config* EventConfig_;
    struct event_base* EventBase_;
    std::unique_ptr<LoopData> Datas_[MAX_EVENTS_SIZE];
    int CurIndex_;
    std::queue<int> IdleIndexs_;
    std::shared_ptr<IO> Wakeup_;
};

}

#endif


