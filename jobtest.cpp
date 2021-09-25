
#include <iostream>

#include "def.h"
#include "jobrecv.h"
#include "jobaccept.h"
#include "jobdisconnect.h"

class TestCallback: public INetworkCallback
{
public:
    TestCallback(){};
	virtual ~TestCallback(){}
	virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port){std::cout<<"OnAccept"<<std::endl;};
	virtual void OnRecv(NetID netid, const char *data, unsigned int length){std::cout<<"OnRecv"<<std::endl;};
	virtual void OnDisconnect(NetID netid){std::cout<<"OnDisconnect"<<std::endl;};
};

int main()
{
    TestCallback m_cb;
    char *buff = (char*)malloc(10*sizeof(char));
    JobAccept j_acc(1, 1, 9999);
    JobRecv j_recv(2, buff, 10);
    JobDisconect j_dis(3);
    j_acc.Invoke((INetworkCallback*)&m_cb);
    j_recv.Invoke((INetworkCallback*)&m_cb);
    j_dis.Invoke((INetworkCallback*)&m_cb);

    return 0;
}
