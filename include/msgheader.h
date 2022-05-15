#pragma once

#pragma pack(push)
#pragma pack(4)

enum {
    MT_INVALID = 0,
};

class MessageHeader {
    public:
        MessageHeader():msg_len(0){}
        MessageHeader(unsigned int len):msg_len(len){}
        unsigned int msg_len;
};

#pragma pack(pop)
