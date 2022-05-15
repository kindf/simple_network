#pragma once

#include "msgheader.h"

#pragma pack(push)
#pragma pack(4)

class MsgTest {
public:
    MsgTest();
    MessageHeader header;
    int msg_type;
    int uid;
    char name[32];
};

#pragma pack(pop)
