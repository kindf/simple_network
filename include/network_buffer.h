#pragma once

#include "base_buffer.h"

#define DEFAULT_RECV_BUFFER_LEN 1024*128

class JobRecv;

class NetworkBuffer: public Buffer {
public:
    NetworkBuffer(const unsigned int size);
    virtual ~NetworkBuffer();
    void BackToPool();
    /* bool HasData()const; */
    unsigned int GetEmptySize() override;

    unsigned int GetWriteSize() const;
    unsigned int GetReadSize() const;
    void FillData(unsigned int size);
    void RemoveData(unsigned int size);
    void ReAllocBuffer();

protected:
    unsigned int m_data_size;
};

class RecvNetworkBuffer: public NetworkBuffer {
public:
    RecvNetworkBuffer(unsigned int size = DEFAULT_RECV_BUFFER_LEN);
    ~RecvNetworkBuffer() = default;

public:
    int GetBuffer(char*& p) const;
    JobRecv* GetPatch(unsigned int netid);
/* private: */
    void MemcpyFromBuffer(char* p, const unsigned int size);
};
