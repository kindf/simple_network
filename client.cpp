
#include <iostream>
#include <cstring>
#include "def.h"
#include "msgmain.h"
#include "network.h"
#include "msgmain.h"
#include <unistd.h>


class TestCallback: public INetworkCallback {
    public:
        virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port){
        };

        virtual void OnRecv(NetID netid, const char *data, unsigned int length) {
            if(count > 5){
                return;
            }
            MsgTest *msg = reinterpret_cast<MsgTest*>(const_cast<char*>(data));
            msg->print();
            net->Send(netid, data, length);
            count++;
            sleep(1);
        };

        virtual void OnDisconnect(NetID netid) {
            cout<<"OnDisconnect"<<endl;
        };
        virtual void OnConnect(bool result, int handler, NetID netid, IP ip, Port port) {
            if(result) {
            }
        };
        void SetNetwork(Network *nett) {net = nett;};
    private:
        Network *net{nullptr};
        int count{0};
};

int main() {
    Network net;
    TestCallback cb;
    cb.SetNetwork(&net);
    net.RegisterCallback(&cb);
    net.Start();
    net.Connect("127.0.0.1", 9999, nullptr, 100);
    while(true) 
    {
        net.Update();
    }
    return 0;
}
