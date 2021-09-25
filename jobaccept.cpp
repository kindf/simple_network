#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "jobaccept.h"

JobAccept::JobAccept(NetID netid, SOCKET sock, Port port):m_netid(netid), m_accept_sock(sock), m_listen_port(port)
{

}

JobAccept::~JobAccept()
{

}

void JobAccept::Invoke(INetworkCallback *callback)
{
    sockaddr_in addr;
    socklen_t len = sizeof(sockaddr_in);
    ::getpeername(m_accept_sock, (struct sockaddr*)&addr, &len);
    IP ip = ntohl(addr.sin_addr.s_addr);
    Port port = ntohs(addr.sin_port);
    //char *ip_str = inet_ntoa(addr.sin_addr);

    callback->OnAccept(m_listen_port, m_netid, ip, port);
}
