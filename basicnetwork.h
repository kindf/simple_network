
#pragma once

#include <vector>
#include <queue>
#include <map>
#include <thread>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <mutex>

#include "def.h"
#include "basicnetworkhandler.h"
#define MAX_EPOLL_SIZE 512
using namespace std;


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
	typedef map<int, RegisterTableItem> RegisterTable;
	typedef map<int, RegisterTableItem>::iterator RegisterTableIter;

	typedef vector<NetID> DirtyQueue;
	typedef queue<Job *> JobTempList;
	typedef vector<BasicNetworkHandler*> HandlerList;

	RegisterTable m_register_table;
    mutex m_register_table_mutex;

	DirtyQueue m_dirty_queue;			//延迟删除项
    mutex m_dirty_queue_mutex;

	thread*	m_work_thread;				//工作线程
	bool m_is_exit;						//线程退出标志

	JobTempList m_job_to_push;
	JobQueue *m_job_queue;
    mutex m_job_queue_mutex;

	void DeleteDirtySocket();
	void PushJobToInvoke();
	static void ThreadFunc(void *param); //线程函数
	void WorkFunc();

	void InitSocket();
	void ReleaseSocket();
	void AddSocket(BasicNetworkHandler* handler);
	void RemoveSocket(SOCKET sock);
	void RegisterSocketWrite(BasicNetworkHandler *handler);
	void UnregisterSocketWrite(BasicNetworkHandler *handler);
	void PollSocket(HandlerList *readhandler, HandlerList *writehandler);
    void Listen(int port);
	
	SOCKET m_epfd;						// epoll fd
	epoll_event m_tmp_event[MAX_EPOLL_SIZE];	// 在PollSocket中使用的临时event数组
    unsigned int m_netid; // netid计数器
};


#include "tcphandler.h"
inline bool BasicNetwork::SendPackage(NetID netid, const char *buffer, unsigned int len) {
    lock_guard<mutex> lk(m_register_table_mutex);
    RegisterTableIter iter = m_register_table.find(netid);
    if(iter == m_register_table.end()) {
        return false;
    }
    TcpHandler *handler = (TcpHandler*)(iter->second.handler);
    if(handler != NULL) {
        if(!handler->SendPackage(buffer, len)) {
            return false;
        }
        return true;
    }
    return false;
}

