
#pragma once

#include <memory>
#include <mutex>
#include <list>

#include "def.h"
#include "basicnetworkhandler.h"
#include "network_buffer.h"

using namespace std;

class WriteObject {
    public:
        streamsize start;
        streamsize len;
        shared_ptr<char> buff;
};

class TcpHandler : public BasicNetworkHandler
{
public:
	TcpHandler(SOCKET socket, int max_package_size);
	TcpHandler(const TcpHandler&) = delete;
	TcpHandler& operator=(const TcpHandler&) = delete;
	~TcpHandler();

	void OnCanRead();
	void OnCanWrite();
	void OnClose();

protected:
	friend class BasicNetwork;
	bool SendPackage(const char *buffer, unsigned int len);

private:

// 发送相关
private:
    list<shared_ptr<WriteObject>> objs;  // 待写块
public:
    void EntireWrite(shared_ptr<char> buff, streamsize len);
    /* void LingerClose(); //全部发完完再关闭 */
    void OnWriteable();
private:
    void EntireWriteWhenEmpty(shared_ptr<char> buff, streamsize len);
    void EntireWriteWhenNotEmpty(shared_ptr<char> buff, streamsize len);
    bool WriteFrontObj();

// 接收相关
private:
    RecvNetworkBuffer* m_recv_buffer{ nullptr };

};
