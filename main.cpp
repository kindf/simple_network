
#include "network.h"
#include "def.h"
#include <iostream>

class TestCallback: public INetworkCallback {
    public:
        TestCallback():net(NULL){};
        virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port){
        };

        virtual void OnRecv(NetID netid, const char *data, unsigned int length) {
            cout<<"OnRecv:"<< data << " len: " << length << endl;
            net->Send(netid, data, length);
        };

        virtual void OnDisconnect(NetID netid) {
            cout<<"OnDisconnect"<<endl;
        };
        void SetNetwork(Network *nett) {net = nett;};
    private:
        Network *net;
};

int main() {
    Network net;
    TestCallback cb;
    cb.SetNetwork(&net);
    net.RegisterCallback(&cb);
    net.Start();
    net.Listen(9999, 5);
    while(true) 
    {
        net.Update();
    }
    return 0;
}
