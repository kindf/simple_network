
#include <vector>
#include "tcphandler.h"
#include "basicnetwork.h"
#include "jobrecv.h"
#include "jobdisconnect.h"
#include "common/memory/msgmem.h"

#define MIN_MSG_TO_WRITE_BUFF 2048

TcpHandler::TcpHandler(SOCKET socket, int max_package_size)
{
	if (m_socket != SOCKET_ERROR)
	{
		// 设为非阻塞
        int flags = ::fcntl(sock, F_GETFL, 0);
        flags |= O_NONBLOCK;
        ::fcntl(sock, F_SETFL, flags);
	}
}


TcpHandler::~TcpHandler()
{
	OnClose();
}

#define MSG_READ_BUFF_LEN	2048

void TcpHandler::OnCanRead()
{
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
			if (ret == SOCKET_ERROR && PISocket::Errno() == EWOULDBLOCK)
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

		while (msg_no_read > 0)
		{
			if (m_cur_read_byte < m_cur_read_block.linklayer.len)
			{
				break;
			}

			if (m_basic_network != 0)
			{
				JobRecv *jobrecv = new JobRecv(m_netid, m_cur_read_block.message, m_cur_read_block.linklayer.len);
				m_basic_network->PushJob(jobrecv);
			}
			else
			{
				delete []m_cur_read_block.message;
			}
		}
	}
}

void TcpHandler::OnCanWrite()
{
	if (m_socket == SOCKET_ERROR)
	{
		return;
	}

	for (;;)
	{
		while(m_cur_write_byte < m_cur_msg_to_write_buff_len)
		{
			int ret = ::send(m_socket, m_msg_to_write_buff + m_cur_write_byte, m_cur_msg_to_write_buff_len - m_cur_write_byte, 0);
			if (ret <= 0)
			{
				if (ret == SOCKET_ERROR && PISocket::Errno() == EWOULDBLOCK)
				{
					return;
				}
				// 出错
				if (m_basic_network != 0)
				{
					m_basic_network->Remove(m_netid);
				}

				return;
			}
		}

		if ( m_basic_network != 0 )
		{
			m_basic_network->UnregisterWrite(m_netid);
		}
	}
}


void TcpHandler::OnClose()
{
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


