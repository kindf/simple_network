#pragma once

#define MAX_SIZE 1024*1024

class Buffer {
public:
    virtual unsigned int GetEmptySize();

    void ReAllocBuffer(unsigned int len);

    unsigned int GetEndIndex() const {
        return m_end_index;
    }

    unsigned int GetBeginIndex() const {
        return m_begin_index;
    }

    unsigned int GetTotalSize() const {
        return m_buffer_size;
    }

protected:
    char* m_buffer{ nullptr };
    unsigned int m_begin_index{0};
    unsigned int m_end_index{0};
    unsigned int m_buffer_size{0};
};

