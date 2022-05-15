#include <iostream>
#include <cstring>
#include "base_buffer.h"

unsigned int Buffer::GetEmptySize() {
    return m_buffer_size - m_end_index;
}

void Buffer::ReAllocBuffer(unsigned int len) {
    if(m_buffer_size >= MAX_SIZE) {
        std::cout << "[Network::Buffer::ReAllocBuffer] size except!!! size:" << m_buffer_size << std::endl;
    }

    char* temp_buff = new char[m_buffer_size * 2];
    unsigned int new_end_index = 0;
    if(m_begin_index < m_end_index) {
        ::memcpy(temp_buff, m_buffer + m_begin_index, m_end_index - m_begin_index);
        new_end_index = m_end_index - m_begin_index;
    } else {
        if(m_begin_index == m_end_index && len <= 0) {
            // 什么也不做
        } else {
            ::memcpy(temp_buff, m_buffer + m_begin_index, m_buffer_size - m_begin_index);
            new_end_index = m_buffer_size - m_begin_index;
            if(m_end_index > 0) {
                :: memcpy(temp_buff + new_end_index, m_buffer, m_end_index);
                new_end_index += m_end_index;
            }
        }
    }

    m_buffer_size *= 2;
    delete[] m_buffer;
    m_buffer = temp_buff;
    m_begin_index = 0;
    m_end_index = new_end_index;
}
