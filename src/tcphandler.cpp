
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <memory.h>
#include <iostream>
#include "tcphandler.h"
#include "basicnetwork.h"
#include "jobrecv.h"
#include "jobdisconnect.h"
#include "def.h"

#define MSG_READ_BUFF_LEN 1024*16

TcpHandler::TcpHandler(SOCKET socket, int max_package_size)
    :BasicNetworkHandler(socket, HT_TCP) {
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
			if (errno == EWOULDBLOCK || errno == EAGAIN)
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
            /* cout << "msg come!!! msg: " << buff << endl; */
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
    OnWriteable();
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

// 把发送内容copy到tcphandler的写buff中
bool TcpHandler::SendPackage(const char *buffer, unsigned int len) {
    auto new_buff = make_shared<char>(len);
    memcpy(new_buff.get(), buffer, len);
    EntireWrite(new_buff, len);
    return true;
}

void TcpHandler::OnWriteable() {
    while(true) {
        if(!WriteFrontObj()) {
            break;
        }
    }

    if(objs.empty()) {
        // 注销写事件
    }
}

bool TcpHandler::WriteFrontObj() {
    if(objs.empty()) {
        return false;
    }

    auto obj = objs.front();
    char *s = obj->buff.get() + obj->start;
    streamsize len = obj->len - obj->start;
    streamsize n = ::write(m_socket, s, len);
    if(n < 0) {
        if(errno == EAGAIN || errno == EINTR) {
            // 写缓冲区满 or 被中断
            return false;
        } else {
            // 出错
            // LingerClose();
            return false;
        }
    } else {
        if(n == len) {
            objs.pop_front();
            return true;
        }

        if(n < len) {
            obj->start = n;
            return false;
        }
    }
    return false;
}

void TcpHandler::EntireWrite(shared_ptr<char> buff, streamsize len) {
    if(objs.empty()) {
        EntireWriteWhenEmpty(buff, len);
    } else {
        EntireWriteWhenNotEmpty(buff, len);
    }
}

void TcpHandler::EntireWriteWhenEmpty(shared_ptr<char> buff, streamsize len) {
    char *s = buff.get();
    streamsize n = ::write(m_socket, s, len);
    streamsize sn = 0;
    if(n < 0) {
        if(errno == EAGAIN || errno == EINTR) {
        } else {
            // 出错
            return;
        }
    } else {
        if(n == len) {
            return;
        }
        if( n < len) {
            sn = n;
        }
    }
    auto obj = make_shared<WriteObject>();
    obj->buff = buff;
    obj->len = len;
    obj->start = sn;
    objs.push_back(obj);
}

void TcpHandler::EntireWriteWhenNotEmpty(shared_ptr<char> buff, streamsize len) {
    auto obj = make_shared<WriteObject>();
    obj->buff = buff;
    obj->len = len;
    obj->start = 0;
    objs.push_back(obj);
}
