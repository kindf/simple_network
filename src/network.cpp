
#include "network.h"
#include "job.h"
#include "listenhandler.h"
#include "tcphandler.h"
#include <memory.h>


class DefaultCallback:public INetworkCallback
{
public:
    virtual ~DefaultCallback(){}
    virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port){}
    virtual void OnRecv(NetID netid, const char *data, unsigned int length){}
    virtual void OnDisconnect(NetID netid){}
};


Network::Network(NetworkConfig config):m_config(config), m_job_queue(), m_basicnetwork(&m_job_queue)
{
    m_default_callback = new DefaultCallback();
    m_callback = m_default_callback;
}

Network::~Network()
{
}

void Network::RegisterCallback(INetworkCallback * callback)
{
    if (callback == 0)
    {
        m_callback = m_default_callback;
    }
    else
    {
        m_callback = callback;
    }
}

void Network::Start()
{
    m_basicnetwork.Start();
}

void Network::Stop()
{
    m_basicnetwork.Stop();
    Job *job;
    lock_guard<mutex> lk(m_job_queue_mutex);
    while (!m_job_queue.empty())
    {
        job = m_job_queue.front();
        m_job_queue.pop();
        delete job;
    }
}

void Network::Update()
{
    Job *job;
    lock_guard<mutex> lk(m_job_queue_mutex);
    while (!m_job_queue.empty())
    {
        job = m_job_queue.front();
        m_job_queue.pop();
        job->Invoke(m_callback);
        delete job;
    }
}

bool Network::Listen(Port port, int backlog, NetID *netid_out, const char *ip_bind)
{
    ListenHandler *listenhandler = new ListenHandler(m_config.max_package_size);
    SOCKET sock = listenhandler->Listen(port, backlog, ip_bind);
    if (sock == SOCKET_ERROR)
    {
        return false;
    }

    NetID netid = m_basicnetwork.Add(listenhandler);

    if (netid_out != 0)
    {
        *netid_out = netid;
    }

    return true;
}

void Network::Disconnect(NetID id)
{
    m_basicnetwork.Remove(id);
}


