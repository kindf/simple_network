
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "tcphandler.h"
#include "basicnetwork.h"
#include "jobrecv.h"
#include "jobdisconnect.h"
#include "def.h"

#define MSG_READ_BUFF_LEN 1024*16

TcpHandler::TcpHandler(SOCKET socket, int max_package_size):BasicNetworkHandler(SOCKET_ERROR, HT_TCP),m_socket(socket) {
    if (m_socket != SOCKET_ERROR)
    {
        // 设为非阻塞
        int flags = ::fcntl(m_socket, F_GETFL, 0);
        flags |= O_NONBLOCK;
        ::fcntl(m_socket, F_SETFL, flags);
    }
}


TcpHandler::~TcpHandler() {
	OnClose();
}

void TcpHandler::OnCanRead() {
	if (m_socket == SOCKET_ERROR)
	{
		return;
	}

	char buff[MSG_READ_BUFF_LEN];

	for (;;)
	{
		int ret = ::recv(m_socket, buff, MSG_READ_BUFF_LEN, 0);
		if (ret <= 0)
		{
			if (ret == EWOULDBLOCK)
			{
				// 读缓冲区空，等待下次可读时间
				return;
			}
			// 出错
			if (m_basic_network != 0)
			{
				m_basic_network->Remove(m_netid);
			}
			return;
		}
        if(m_basic_network != 0){
            JobRecv *jobrecv = new JobRecv(m_netid, buff, ret);
            m_basic_network->PushJob(jobrecv);
        }
	}
}

void TcpHandler::OnCanWrite() {
	if (m_socket == SOCKET_ERROR)
	{
		return;
	}


	if ( m_basic_network != 0 )
	{
		m_basic_network->UnregisterWrite(m_netid);
	}
}


void TcpHandler::OnClose() {
	if (m_socket != SOCKET_ERROR)
	{
		::shutdown(m_socket, SHUT_RDWR);
		::close(m_socket);
		
		if (m_basic_network != 0)
		{
			JobDisconect *jobdisconnect = new JobDisconect(m_netid);
			m_basic_network->PushJob(jobdisconnect);
		}

		m_socket = SOCKET_ERROR;

	}
	// 从此处开始m_socket已经无效
}


