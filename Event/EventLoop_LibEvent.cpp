#include <event2/event.h>
#include <sys/eventfd.h>
#include "EventLoop_LibEvent.h"
#include "Exception.h"
#include "IO.h"
#include "Macro.h"

namespace Essie{

//auto gEvFn = std::bind(&EventLoopL::EventLoopCallBack, (EventLoopL*)nullptr, -1, std::placeholders::_1);
struct LoopData
{
	LoopData(std::function<void(int)> &&f) : ev(NULL), fn(std::move(f)){}
	LoopData() = delete;
	LoopData(std::function<void(int)>& f) = delete;

	struct event* ev;
	std::weak_ptr<IO> io;
	std::function<void(int)> fn;
};

static void CEventCallBack(evutil_socket_t fd, short cond, void * arg)
{
	std::function<void(int)> *fn = static_cast< std::function<void(int)>* >(arg);
	(*fn)(cond);
}

EventLoopL::EventLoopL()
	:Creator_(std::this_thread::get_id()),
	EventConfig_(NULL),
	EventBase_(NULL),
	CurIndex_(0)
{
}

EventLoopL::~EventLoopL()
{
	for(auto &i : Datas_)
	{
		if(i)
		{
			RemoveIO(i->io);
		}
	}

	if(EventConfig_)
	{
		event_config_free(EventConfig_);
		EventConfig_ = NULL;
	}

	if(EventBase_)
	{
		event_base_free(EventBase_);
		EventBase_= NULL;
	}
}

bool EventLoopL::Prepare()
{
	_DBG("EventLoopL::Prepare");

	if(!EventConfig_)
	{
		EventConfig_ = event_config_new();
	}

	if(!EventConfig_)
	{
		throwSystemError("event_config_new allocate fail!");
	}

	//event_config_avoid_method(EventConfig_, "select");//avoid select
	//event_config_avoid_method(EventConfig_, "poll");//avoid poll
	if(0 != event_config_require_features(EventConfig_, EV_FEATURE_O1))//EV_FEATURE_ET ?
	{
		_ERR("set event config feature fail!");
		return false;
	}

	if(0 != event_config_set_flag(EventConfig_, EVENT_BASE_FLAG_NOLOCK))
	{
		_ERR("set event config flag fail!");
		return false;
	}

	if(!EventBase_)
	{
		EventBase_ = event_base_new_with_config(EventConfig_);
	}

	if(!EventBase_)
	{
		throwSystemError("event_base_new_with_config allocate fail!");
		return false;
	}

	_DBG("Current used method : %s", event_base_get_method(EventBase_));

	for(auto &data : Datas_)
	{
		if(data)
		{
			std::shared_ptr<IO> io(data->io.lock());
			if(!io)
			{
				_WRN("This io %d is not exist!", io->Index());
				continue;
			}

			if(io->Index() < 0)
			{
				_ERR("This IO has been removed!");
				continue;
			}
			
			if(data->ev)
			{
				event_del(data->ev);
				event_free(data->ev);
				data->ev = NULL;
			}

			event* ev = event_new(EventBase_, io->Fd(), io->Condition(), CEventCallBack, (void*)&(data->fn));	
			_DBG("Add io %p(index : %d  fd : %d  cond : %d)", ev, io->Index(), io->Fd(), io->Condition());

			if(ev == NULL)
			{
				_ERR("event_new fail!");
				continue;
			}
			data->ev = ev;
		}
	}

	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0)
	{
		_ERR("Get eventfd fail!");
		return false;
	}
	Wakeup_.reset(new IO(evtfd, EV_READ | EV_PERSIST));
	Wakeup_->SetCallback(std::bind(&EventLoopL::quitAsync, this, std::placeholders::_1));
	AddIO(Wakeup_);

	return true;
}

bool EventLoopL::AddIO(std::shared_ptr<IO>& io)
{
	if(std::this_thread::get_id() != Creator_)
	{
		_ERR("You can only add IO in the thread that creates this event loop!");
 		return false;
	}

	if(io->Index() >= 0)
	{
		_ERR("This IO has been added!");
 		return false;
	}

	int idx = -1;
	if(IdleIndexs_.empty())
	{
		if(CurIndex_ < MAX_EVENTS_SIZE)
		{
			idx = CurIndex_++;
		}
		else
		{
			_ERR("Event io is full!");
			return false;
		}
	}
	else
	{
		idx = IdleIndexs_.front();
		IdleIndexs_.pop();
	}

	if(idx < 0)
	{
		_ERR("Get index fail!");
 		return false;
	}

	if(Datas_[idx] && Datas_[idx]->ev)
	{
		event_del(Datas_[idx]->ev);
		event_free(Datas_[idx]->ev);
		Datas_[idx]->ev = NULL;
	}

	auto f = std::bind(&EventLoopL::EventLoopCallBack, this, idx, std::placeholders::_1);
	Datas_[idx].reset(new LoopData(std::move(f)));
	Datas_[idx]->io = io;
	io->SetIndex(idx);
	
	if(EventBase_ != NULL)
	{
		event* ev = event_new(EventBase_, io->Fd(), io->Condition(), CEventCallBack, (void*)&(Datas_[idx]->fn));	
		_DBG("Add io %p(index : %d  fd : %d  cond : %d)", ev, io->Index(), io->Fd(), io->Condition());
		if(ev == NULL)
		{
			_ERR("event_new fail!");
			return false;
		}
		Datas_[idx]->ev = ev;

		return event_add(ev, NULL) ? false : true;
	}

	return true;
}

void EventLoopL::RemoveIO(std::weak_ptr<IO>& wio)
{
	if(std::this_thread::get_id() != Creator_)
	{
		_ERR("You can only remove IO in the thread that creates this event loop!");
 		return;
	}

	std::shared_ptr<IO> io(wio.lock());
	if(!io)
	{
		_ERR("This io is not exist!");
		return;
	}

	if(io->Index() < 0)
	{
		_ERR("This IO has been removed!");
		return;
	}

	event_del(Datas_[io->Index()]->ev);
	event_free(Datas_[io->Index()]->ev);
	Datas_[io->Index()]->ev = NULL;
	IdleIndexs_.push(io->Index());
	io->SetIndex(-1);
}

void EventLoopL::Loop()
{
	event_base_dispatch(EventBase_);
}

void EventLoopL::Quit()
{
	if(std::this_thread::get_id()  == Creator_)
	{
		_DBG("Quit!");
		event_base_loopexit(EventBase_, NULL);
		//or event_base_loopbreak(EventBase_);
	}
	else
	{
		_DBG("Quit async!");
		if(Wakeup_)
		{
			eventfd_t quit = 1;
			eventfd_write(Wakeup_->Fd(), quit);
		}
	}
}

void EventLoopL::quitAsync(int cond)
{
	eventfd_t quit = 1;
	eventfd_read(Wakeup_->Fd(), &quit);

	event_base_loopexit(EventBase_, NULL);
	//or event_base_loopbreak(EventBase_);
}

void EventLoopL::EventLoopCallBack(int index, short cond)
{
	_DBG("Current index is %d", index);
	if(index < 0)
	{
		_ERR("Current index is %d, below 0", index);
		return;
	}

	std::shared_ptr<IO> io = Datas_[index]->io.lock();
	if(io)
	{
		io->HandleEvent(cond);
	}
	else
	{
		_PRI("Current index[%d]'s IO has been removed, delete related event", index);
		event_del(Datas_[index]->ev);
		event_free(Datas_[index]->ev);
		Datas_[index]->ev = NULL;
		IdleIndexs_.push(index);
	}
}

}

