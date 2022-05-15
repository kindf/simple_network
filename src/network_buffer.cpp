#include <iostream>
#include <cstring>
#include "network_buffer.h"
#include "msgheader.h"
#include "jobrecv.h"

NetworkBuffer::NetworkBuffer(unsigned int size) {
    m_buffer_size = size;
    m_data_size = 0;
    m_buffer = new char[size];
    // 检查m_buffer合法性
}

NetworkBuffer::~NetworkBuffer() {
    if(m_buffer != nullptr) {
        delete[] m_buffer;
    }
}

void NetworkBuffer::BackToPool() {
    m_begin_index = 0;
    m_end_index = 0;
    m_data_size = 0;
}

unsigned int NetworkBuffer::GetWriteSize() const {
    if(m_begin_index <= m_end_index) {
        return m_buffer_size - m_end_index;
    } else {
        return m_begin_index - m_end_index;
    }
}

unsigned int NetworkBuffer::GetReadSize() const {
    if(m_data_size <= 0) {
        return 0;
    }
    if(m_begin_index < m_end_index) {
        return m_end_index - m_begin_index;
    } else {
        return m_buffer_size - m_begin_index;
    }
}

unsigned int NetworkBuffer::GetEmptySize() {
    return m_buffer_size - m_data_size;
}


void NetworkBuffer::FillData(unsigned int size) {
    m_data_size += size;
    // 更新m_end_index位置
    if((m_buffer_size - m_end_index) <= size) {
        size -= m_buffer_size - m_end_index;
        m_end_index = 0;
    }
    m_end_index += size;
}

void NetworkBuffer::RemoveData(unsigned int size) {
    m_data_size -= size;
    if((m_begin_index + size) >= m_buffer_size) {
        size -= m_buffer_size - m_begin_index;
        m_begin_index = 0;
    }
    m_begin_index += size;
}

void NetworkBuffer::ReAllocBuffer(){
    Buffer::ReAllocBuffer(m_data_size);
}

////////////////////////////////////////////////////////////////////////////////////////

RecvNetworkBuffer::RecvNetworkBuffer(unsigned int size): NetworkBuffer(size) {
}

int RecvNetworkBuffer::GetBuffer(char*& p) const {
    p = m_buffer + m_end_index;
    return GetWriteSize();
}

JobRecv* RecvNetworkBuffer::GetPatch(unsigned int netid) {
    // 协议头部长度不足
    if(m_data_size < sizeof(MessageHeader)) {
        return nullptr;
    }

    MessageHeader header;
    MemcpyFromBuffer(reinterpret_cast<char*>(&header), sizeof(MessageHeader));
    // 协议长度不足
    if(m_data_size < header.msg_len) {
        return nullptr;
    }
    RemoveData(sizeof(MessageHeader));
    char temp[header.msg_len+1];
    MemcpyFromBuffer(temp, header.msg_len);
    RemoveData(header.msg_len);
    JobRecv *jobrecv = new JobRecv(netid, temp, header.msg_len);
    return jobrecv;
}


void RecvNetworkBuffer::MemcpyFromBuffer(char* p, const unsigned int size) {
    const auto read_size = GetReadSize();
    if(read_size < size) {
        ::memcpy(p, m_buffer + m_begin_index, read_size);
        ::memcpy(p + read_size, m_buffer, size - read_size);
    } else {
        ::memcpy(p, m_buffer + m_begin_index, read_size);
    }
}
