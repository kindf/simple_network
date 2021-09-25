
#ifndef JOBRECV_H
#define JOBRECV_H

#include "def.h"
#include "job.h"

class MsgMem;

class JobRecv :public Job
{
public:
	JobRecv(NetID netid, char *data, unsigned int len);
	virtual ~JobRecv();
	virtual void Invoke(INetworkCallback *callback);

    // TODO: 内存池
	// void *operator new(size_t c);
	// void operator delete(void *m);
protected:
	char         *m_data;
	unsigned int m_length;
	NetID        m_netid;
};


#endif


