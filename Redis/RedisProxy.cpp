#include "Macro.h"
#include "RedisProxy.h"

namespace Stone {

thread_local redisAsyncContext* RedisProxy::RAC_ = nullptr;
thread_local bool RedisProxy::Connected_ = false;
thread_local std::shared_ptr<IO> RedisProxy::ReadIO_;
thread_local std::shared_ptr<IO> RedisProxy::WriteIO_;

RedisProxy::RedisProxy():Port_(0)
{
}

RedisProxy::~RedisProxy()
{
}

RedisProxy& RedisProxy::GetInstance()
{
	static RedisProxy proxy;
	return proxy;
}

bool RedisProxy::Init(const std::string& ip, int port)
{
	IP_ = ip;
	Port_ = port;

	return true;
}

bool RedisProxy::Connect()
{
	_DBG("Connect begin...");
	if(Connected_)
	{
		_DBG("Redis Proxy has been connected.");
		return true;
	}

	std::string ip = "127.0.0.1";
	if(!IP_.empty())
	{
		ip = IP_;	
	}
	int port = 6379;
	if(Port_ > 0)
	{
		port = Port_;
	}

	RAC_ = redisAsyncConnect(ip.c_str(), port);
	if(!RAC_)
	{
		_ERR("allocate redis async connection fail!");
		return false;
	}

	if(RAC_->err != 0)
	{
		_ERR("connect fail[%s]", RAC_->errstr);
		redisAsyncFree(RAC_);
		RAC_ = nullptr;
		return false;
	}

	RAC_->data = this;
	RAC_->ev.addRead = RedisProxy::redisLibeventAddRead;
	RAC_->ev.delRead = RedisProxy::redisLibeventDelRead;
	RAC_->ev.addWrite = RedisProxy::redisLibeventAddWrite;
	RAC_->ev.delWrite = RedisProxy::redisLibeventDelWrite;
	RAC_->ev.cleanup = RedisProxy::redisLibeventCleanup;

	ReadIO_.reset(new IO(RAC_->c.fd, EV_READ));
	WriteIO_.reset(new IO(RAC_->c.fd, EV_WRITE));
	ReadIO_->SetCallback(
			std::bind(&RedisProxy::handleRead,
				this,
				std::placeholders::_1));

	WriteIO_->SetCallback(
			std::bind(&RedisProxy::handleWrite,
				this,
				std::placeholders::_1));

	redisAsyncSetConnectCallback(RAC_, RedisProxy::handleConnect);
	redisAsyncSetDisconnectCallback(RAC_, RedisProxy::handleDisconnect);
	Connected_ = LoopThread::AddLoopIOLocal(ReadIO_)
		&& LoopThread::AddLoopIOLocal(WriteIO_);

	return Connected_;
}

void RedisProxy::redisLibeventAddRead(void *privdata)
{
	_DBG("RedisProxy::redisLibeventAddRead");
	RedisProxy* pThis = static_cast<RedisProxy*>(privdata);
	LoopThread::RestartLoopIOLocal(pThis->ReadIO_);
}

void RedisProxy::redisLibeventDelRead(void *privdata)
{
	_DBG("RedisProxy::redisLibeventDelRead");
	RedisProxy* pThis = static_cast<RedisProxy*>(privdata);
	LoopThread::FreezeLoopIOLocal(pThis->ReadIO_);
}

void RedisProxy::redisLibeventAddWrite(void *privdata)
{
	_DBG("RedisProxy::redisLibeventAddWrite");
	RedisProxy* pThis = static_cast<RedisProxy*>(privdata);
	bool ret = LoopThread::RestartLoopIOLocal(pThis->WriteIO_);
	_DBG("%d", ret);
}

void RedisProxy::redisLibeventDelWrite(void *privdata)
{
	_DBG("RedisProxy::redisLibeventDelWrite");
	RedisProxy* pThis = static_cast<RedisProxy*>(privdata);
	LoopThread::FreezeLoopIOLocal(pThis->WriteIO_);
}

void RedisProxy::redisLibeventCleanup(void *privdata)
{
	_DBG("RedisProxy::redisLibeventCleanup");
	RedisProxy* pThis = static_cast<RedisProxy*>(privdata);
	LoopThread::FreezeLoopIOLocal(pThis->ReadIO_);
	LoopThread::FreezeLoopIOLocal(pThis->WriteIO_);
}

void RedisProxy::handleRead(int cond)
{
	_DBG("handleRead");
}

void RedisProxy::handleWrite(int cond)
{
	_DBG("handleWrite");
	redisAsyncHandleWrite(RAC_);
}

void RedisProxy::handleConnect(const redisAsyncContext *c, int status)
{
	_DBG("handleConnect status:%d", status);

}

void RedisProxy::handleDisconnect(const redisAsyncContext *c, int status)
{
	_DBG("handleDisconnect");
}
}
