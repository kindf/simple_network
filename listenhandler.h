
#pragma once

#include "def.h"
#include "basicnetworkhandler.h"

class ListenHandler : public BasicNetworkHandler
{
	
public:
	ListenHandler(int accept_tcp_max_pack_szie);
	~ListenHandler();

	int Listen(int port, int backlog, const char *ip_bind=0);

	void OnCanRead();
	void OnClose();

protected:

	int m_listen_port;
	void Close();

private:
	ListenHandler(const ListenHandler&);
	ListenHandler& operator=(const ListenHandler&);

	int m_accept_tcp_max_package_size;
};

