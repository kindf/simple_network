
#ifndef BASICNETWORK_H
#define BASICNETWORK_H

#include <vector>
#include <queue>
#include <map>

#include "def.h"
#include "basicnetworkhandler.h"
#include "jobqueue.h"


class Job;

class BasicNetwork
{
public:
	BasicNetwork(JobQueue *job_queue);
	~BasicNetwork();

	void Start();
	void Stop();

	NetID Add(BasicNetworkHandler *handler);
	void Remove(NetID netid);
	void Clear();

	void Print();

	bool SendPackage(NetID netid, const char *data, unsigned int len);

protected:
	friend class ListenHandler;
	friend class TcpHandler;
	void PushJob(Job * job);
	bool UnregisterWrite(NetID netid, int num=1);

private:
	struct RegisterTableItem
	{
		BasicNetworkHandler *handler;
		int write_event;

		RegisterTableItem():write_event(0){}
	};
	typedef std::map<RegisterTableItem> RegisterTable;
	typedef std::map<RegisterTableItem>::iterator RegisterTableIter;

	typedef std::vector<NetID> DirtyQueue;
	typedef std::queue<Job *> JobTempList;
	typedef std::vector<BasicNetworkHandler*> HandlerList;

	RegisterTable m_register_table;
    std::mutex m_register_table_mutex;

	DirtyQueue m_dirty_queue;			//延迟删除项
    std::mutex m_dirty_queue_mutex;

	Thread	m_work_thread;				//工作线程
	bool m_is_exit;						//线程退出标志

	JobTempList m_job_to_push;
	JobQueue *m_job_queue;

	void DeleteDirtySocket();
	void PushJobToInvoke();
	static DWORD ThreadFunc(void *param); //线程函数
	DWORD WorkFunc();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// 以下为平台网络io模型相关代码 ////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	void InitSocket();
	void ReleaseSocket();
	void AddSocket(BasicNetworkHandler* handler);
	void RemoveSocket(SOCKET sock);
	void RegisterSocketWrite(BasicNetworkHandler *handler);
	void UnregisterSocketWrite(BasicNetworkHandler *handler);
	void PollSocket(HandlerList *readhandler, HandlerList *writehandler);
	
	SOCKET m_epfd;						// epoll fd
	epoll_event m_tmp_event[MAX_EPOLL_SIZE];	// 在PollSocket中使用的临时event数组
    unsigned int m_netid; // netid计数器
};


#include "tcphandler.h"
// 每秒几十W次
inline bool BasicNetwork::SendPackage(NetID netid, const char *buffer, unsigned int len)
{
	m_register_table_mutex.lock();

	RegisterTableIter iter = m_register_table.find(netid);
	if (iter == m_register_table.end())
	{
		m_register_table_mutex.unlock();
		return false;
	}

	bool ret = false;
	TcpHandler *tcphandler = (TcpHandler*)(iter->handler);
	if (tcphandler != 0 && tcphandler->GetType() == BasicNetworkHandler::HT_TCP)
	{
		if (!tcphandler->SendPackage(buffer, len))
		{
			m_register_table_mutex.unlock();
			return false;	
		}
		
		if (iter->write_event == 0)
		{
			RegisterSocketWrite(tcphandler);
		}
		iter->write_event += len + sizeof(LinkLayer);
		ret = true;
	}
	m_register_table_mutex.unlock();

	return ret;
}

#endif

