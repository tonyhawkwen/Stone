#ifndef _STONE_REDIS_PROXY_H_
#define _STONE_REDIS_PROXY_H_

#include <hiredis/hiredis.h> 
#include <hiredis/async.h>
#include <event.h>
#include <memory>
#include <thread>
#include <string>
#include "LoopThread.h"
#include "IO.h"

namespace Stone {

class RedisProxy
{
public:
	~RedisProxy();
	
	static RedisProxy& GetInstance();
	bool Init(const std::string& ip, int port);
	bool Connect();
	bool SendRequest();
	void Close();

private:
	RedisProxy();
	static void redisLibeventAddRead(void *privdata);
	static void redisLibeventDelRead(void *privdata);
	static void redisLibeventAddWrite(void *privdata);
	static void redisLibeventDelWrite(void *privdata);
	static void redisLibeventCleanup(void *privdata);

	void handleRead(int cond);
	void handleWrite(int cond);
	
	static void handleConnect(const redisAsyncContext *c, int status);
	static void handleDisconnect(const redisAsyncContext *c, int status);

	std::string IP_;
	int Port_;
	static thread_local redisAsyncContext* RAC_;
	static thread_local bool Connected_;
	static thread_local std::shared_ptr<IO> ReadIO_;
	static thread_local std::shared_ptr<IO> WriteIO_;

};

}

#endif
