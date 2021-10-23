
#pragma once

#include "def.h"
#include "basicnetwork.h"
#include <queue>

class INetworkCallback;
class job;

class Network
{
public:
	Network(NetworkConfig config=NetworkConfig());
	~Network();

	void RegisterCallback(INetworkCallback * callback);
	void Start();
	void Stop();

	void Update();

	// Listen 和 Connect都只能在Start之前调用
	bool Listen(Port port, int backlog, NetID *netid_out=0, const char *ip_bind=0);
	void Disconnect(NetID id);

	bool Send(NetID netid, const char *data, unsigned int len);
protected:
	NetworkConfig m_config;
    JobQueue m_job_queue;
	BasicNetwork m_basicnetwork;
	INetworkCallback *m_callback;
	INetworkCallback *m_default_callback;

};

// 每秒几十W次
inline bool Network::Send(NetID netid, const char *data, unsigned int len)
{	
	return m_basicnetwork.SendPackage(netid, data, len);
}

