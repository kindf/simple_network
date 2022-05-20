
#include "network.h"
#include "job.h"
#include "listenhandler.h"
#include "tcphandler.h"
#include <memory.h>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

class DefaultCallback:public INetworkCallback
{
public:
    virtual ~DefaultCallback(){}
    virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port){}
    virtual void OnRecv(NetID netid, const char *data, unsigned int length){}
    virtual void OnDisconnect(NetID netid){}
    virtual void OnConnect(bool result, int handler, NetID netid, IP ip, Port port){}
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
    Job *job = NULL;
    lock_guard<mutex> lk(m_job_queue_mutex);
    while (!m_job_queue.empty())
    {
        job = m_job_queue.front();
        if(job != NULL) {
            m_job_queue.pop();
            delete job;
        }
    }
}

void Network::Update()
{
    Job *job = NULL;
    lock_guard<mutex> lk(m_job_queue_mutex);
    while (!m_job_queue.empty())
    {
        job = m_job_queue.front();
        m_job_queue.pop();
        if(job != NULL) {
            job->Invoke(m_callback);
            delete job;
        }
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

bool Network::Connect(const char* ip, unsigned short port, unsigned int* net_id, unsigned long time_out) {
    bool ret = true;
    NetID netid;
    IP ip_host;
    do {
        unsigned long ip_n = inet_addr(ip);
        if(ip_n == INADDR_NONE) {
            ret = false;
            break;
        }
        ip_host = ntohl(ip_n);
        SOCKET sock = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if( sock == INVALID_SOCKET ){
            cout << "[Network::Connect] connect socket create failed" << endl;
            ret = false;
            break;
        }
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(ip_host);
        addr.sin_port = htons(port);

        /* int flags = ::fcntl(sock, F_GETFL, 0); */
        /* flags |= SOCK_NONBLOCK; */
        /* if ( SOCKET_ERROR == ::fcntl(sock, F_SETFL, flags) ) */
        /* { */
        /*     ::close(sock); */
        /*     cout << "[Network::Connect] set nonblock failed" << endl; */
        /*     return false; */
        /* } */

        if(::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            cout << "[Network::Connect] connect failed" << endl;
            ::close(sock);
            ret = false;
            break;
        }
        // connect成功
        TcpHandler *h = new TcpHandler(sock, m_config.max_package_size);
        netid = m_basicnetwork.Add(h);
        if(net_id != nullptr) {
            *net_id = netid;
        }
    }while(false);
    m_callback->OnConnect(ret, 0, netid, ip_host, port);
    return ret;
}

