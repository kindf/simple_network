

#ifndef JOBDISCONNECT_H
#define JOBDISCONNECT_H

#include "job.h"

class JobDisconect:public Job
{
public:
	JobDisconect(NetID netid);
	virtual ~JobDisconect();

	virtual void Invoke(INetworkCallback *callback);

    // TODO: 内存池
	// void *operator new(size_t c);
	// void operator delete(void *m);
protected:
	NetID m_netid;
};


#endif

