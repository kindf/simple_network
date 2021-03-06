
#include <iostream>
#include <cstring>
#include "def.h"
#include "msgmain.h"
#include "network.h"


class TestCallback: public INetworkCallback {
    public:
        TestCallback():net(NULL){};
        virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port){
            MsgTest msg;
            msg.msg_type = 1;
            msg.uid = 11;
            char name[32] = "liang xiao lu";
            ::memcpy(msg.name, name, sizeof(name));
            msg.header.msg_len = sizeof(MsgTest);
            /* msg.header.msg_len = sizeof(MsgTest) - sizeof(MessageHeader); */
            net->Send(netid, reinterpret_cast<char*>(&msg), sizeof(MsgTest));
        };

        virtual void OnRecv(NetID netid, const char *data, unsigned int length) {
            /* cout<<"OnRecv:"<< data << " len: " << length << endl; */
            MsgTest *msg = reinterpret_cast<MsgTest*>(const_cast<char*>(data));
            msg->print();
            net->Send(netid, data, length);
        };

        virtual void OnDisconnect(NetID netid) {
            cout<<"OnDisconnect"<<endl;
        };
        virtual void OnConnect(bool result, int handler, NetID netid, IP ip, Port port) {}
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
