//
// Created by 白杰 on 2024/4/22.
//

#ifndef MYSQL_REPLICATER_BINLOGSERVER_H
#define MYSQL_REPLICATER_BINLOGSERVER_H

#include <stdio.h>
#include <string>
using namespace std;

class Parser {
    int Iint();
    int GetMySQLServerID();
    int GetMySQLBinlogPos();
    int GetMySQLBinlogFlags();
    int GetMySQLBinlogFileName();
};

struct binlogEvent {
    uint8_t ServerId;
    string dataSource;
    string binlogFileName;
    string currentBinlogFileName;
    uint8_t binlogPosition;
    uint32_t binlogTimestamp;
    uint64_t lastEventID;
    string maxBinlogFileName;
    uint32_t maxBinlogPosition;
    string connectionId;
    bool binlog_checksum;
    bool filterNextRowEvent;
    bool isGTID;
    uint64_t nextEventID;
    string dbType;


};


#endif //MYSQL_REPLICATER_BINLOGSERVER_H
