
#include "def.h"
#include "jobdisconnect.h"

JobDisconect::JobDisconect(NetID netid): m_netid(netid)
{

}

JobDisconect::~JobDisconect()
{

}

void JobDisconect::Invoke(INetworkCallback *callback)
{
	//回调
	callback->OnDisconnect(m_netid);
}
