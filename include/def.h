
#pragma once

#include <queue>
/* #include <function> */

class Job;

typedef	unsigned int		NetID;
typedef unsigned int		IP;
typedef unsigned short		Port;
typedef int					MsgLen;
typedef int                 SOCKET;
typedef std::queue<Job*>    JobQueue;
#define SOCKET_ERROR        -1
#define INVALID_SOCKET      -1

class INetworkCallback
{
public:
	virtual ~INetworkCallback(){}
	virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port)=0;
	virtual void OnRecv(NetID netid, const char *data, unsigned int length)=0;
	virtual void OnDisconnect(NetID netid)=0;
    virtual void OnConnect(bool result, int handler, NetID netid, IP ip, Port port) = 0;
};

/* typedef std::function<void(Port, NetID, IP, Port)> OnAcceptFunc; */

struct NetworkConfig
{
	NetworkConfig():job_queue_length(1024 * 16),			// 64K
		max_package_size(1024 * 1024)
	{

	}
	int job_queue_length;
	int max_package_size;
};

