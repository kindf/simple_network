
#include <cstring>
#include "jobrecv.h"

JobRecv::JobRecv(NetID netid, char *data, unsigned int len):m_length(len), m_netid(netid) {
    auto buff = new char[len+1];
    m_data = buff;
    memcpy(m_data, data, len);
}

JobRecv::~JobRecv() {
    delete m_data;
}

void JobRecv::Invoke(INetworkCallback *callback) {
    // 回调
    callback->OnRecv(m_netid, m_data, (unsigned int)m_length);
}
