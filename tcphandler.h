
#pragma once

#ifndef TCPHANDLER_H
#define TCPHANDLER_H

#include <memory.h>

#include "def.h"
#include "basicnetworkhandler.h"

class TcpHandler : public BasicNetworkHandler
{
public:
	TcpHandler(SOCKET socket, int max_package_size);
	~TcpHandler();

	void OnCanRead();
	void OnCanWrite();
	void OnClose();

protected:
	friend class BasicNetwork;
	bool SendPackage(const char *buffer, unsigned int len)
	{
		return true;
	}

private:
	TcpHandler(const TcpHandler&);
	TcpHandler& operator=(const TcpHandler&);

    int m_socket;
};


#endif
