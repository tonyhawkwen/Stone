#ifndef _STONE_TCP_CHANNEL_H_
#define _STONE_TCP_CHANNEL_H_

#include <unistd.h>
#include <fcntl.h>
#include <memory>
#include "Noncopyable.h"
#include "IO.h"

namespace Stone{

struct InetAddress
{
	uint32_t addr;
	std::string saddr;
	uint16_t port;
};

class TcpChannel : private Noncopyable
{
public:
	typedef std::function<void(int sockfd, const InetAddress&)> ReadCallback;

	TcpChannel():
		Listenning_(false),
		IdleFd_(open("/dev/null", O_RDONLY | O_CLOEXEC))
		{
		}

	~TcpChannel()
	{
		close(IdleFd_);
	}

	bool Create();
	bool IsListenning(){return Listenning_;}
	void Listen();
	std::shared_ptr<IO>& TcpIO(){return TcpIO_;}
	void SetReadCallback(const ReadCallback& cb){Callback_ = std::move(cb);}

private:
	void handleRead(int cond);
	
	std::shared_ptr<IO> TcpIO_;
	bool Listenning_;
	ReadCallback Callback_;
	int IdleFd_;
};

};

#endif
