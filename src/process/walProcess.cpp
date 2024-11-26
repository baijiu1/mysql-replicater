//
// Created by 白杰 on 2024/6/25.
//

#include <cstdio>
#include "walProcess.h"

using namespace std;

int startWalParser(vector<uint8_t> data) {
    struct XLogPageHeaderData * xlogPageHeaderData = (struct XLogPageHeaderData *) malloc(sizeof(struct XLogPageHeaderData) * 2);
    for(uint8_t bytes:data) {

    }
}




// 每个线程开始读取wal日志，一个线程处理一个page(8k)
void startWalProcess(int threadId, vector<uint8_t> data) {
    if (threadId == 0) {
        startXLogLongPageHeaderDataProcess(data);
    }
    startXLogPageHeaderDataProcess(data);
};

// handle for XLogLongPageHeaderData
int startXLogLongPageHeaderDataProcess(const vector<uint8_t>& data) {



//    for (int i = 0; i < data.size(); ++i) {
//        printf(" %#x ", data[i]);
//    }
}


int startXLogPageHeaderDataProcess(const vector<uint8_t>& data) {
    for (int i = 0; i < data.size(); ++i) {
        printf(" %#x ", data[i]);
    }
}