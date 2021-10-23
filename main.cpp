
#include "network.h"
#include "def.h"
#include <iostream>

class TestCallback: public INetworkCallback {
	virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port){cout<<"OnAccept"<<endl;};
	virtual void OnRecv(NetID netid, const char *data, unsigned int length){cout<<"OnRecv"<<endl;};
	virtual void OnDisconnect(NetID netid){cout<<"OnDisconnect"<<endl;};
};

int main() {
    TestCallback cb;
    Network net;
    net.RegisterCallback(&cb);
    net.Start();
    net.Listen(9999, 5);
    while(true) 
    {
        net.Update();
    }
    return 0;
}
