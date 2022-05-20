#pragma once

#include "msgheader.h"
#include <iostream>

using namespace std;

#pragma pack(push)
#pragma pack(4)

class MsgTest {
public:
    MsgTest();
    MessageHeader header;
    int msg_type;
    int uid;
    char name[32];
public:
    void print() {
        cout << "[MsgTest::Print] " << "header:" << header.msg_len << endl;
        cout << "[MsgTest::Print] " << "msg_type:" << msg_type << endl;
        cout << "[MsgTest::Print] " << "uid:" << uid << endl;
        cout << "[MsgTest::Print] " << "name:" << name << endl;
    }
};

#pragma pack(pop)
