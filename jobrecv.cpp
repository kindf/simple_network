
#include "jobrecv.h"

JobRecv::JobRecv(NetID netid, char *data, unsigned int len):m_data(data), m_length(len), m_netid(netid)
{

}

JobRecv::~JobRecv()
{
    delete []m_data;
}

void JobRecv::Invoke(INetworkCallback *callback)
{
    // 回调
    callback->OnRecv(m_netid, (const char *)m_data, (unsigned int)m_length);
}
