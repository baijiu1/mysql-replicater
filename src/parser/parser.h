//
// Created by 白杰 on 2024/5/7.
//

#ifndef MYSQL_REPLICATER_PARSER_H
#define MYSQL_REPLICATER_PARSER_H
#include <string>
#include <map>
#include "binlogEvent_test.h"

using namespace std;

class Parser {
public:
    int parserQuery(char *RecvBuffer, int i);
    int* readRows(int * recvSize, uint8_t *data);
    struct ComQueryFieldPayload* readColumns(vector<uint8_t> *fieldsData);

    int handleBufferHeader(const char *RecvBuffer, int recvSize, uint8_t *data);
    int handleBufferFields(int *sizeData, uint8_t *data);

    pair<int, vector<uint8_t> > SkipLengthEnodedString(struct ComQueryFieldPayload *memFieldPayload, int *recvSize, uint8_t *data, int pos);
    pair<int, vector<uint8_t> > LengthEnodedString(struct ComQueryFieldPayload *memFieldPayload, int *recvSize, uint8_t *data, int pos);
    int LengthEncodedInt(uint8_t *data);

    pair<int, vector<uint8_t> > takerFieldData(struct ComQueryFieldPayload *memFieldPayload, int num, int *recvSize, uint8_t *data, int pos);
    pair<int, vector<uint8_t> > SkipLengthEnodedStringRows(int *recvSize, uint8_t *data, int pos);
    int LengthEncodedRowsInt(uint8_t *data);
    pair<int, vector<uint8_t> > takerFieldRowsData(int num, int *recvSize, uint8_t *data, int pos);
    pair<int, vector<uint8_t> > LengthEnodedRowsString(int *recvSize, uint8_t *data, int pos);

    pair<int, vector<uint8_t> > LengthEnodedRowsStringTest(int *recvSize, uint8_t *data, int pos, int fieldI);
    pair<int, vector<uint8_t> > takerFieldRowsDataTest(int num, int *recvSize, uint8_t *data, int pos, int fieldI);

    // 解析binlog部分
    int handleBufferBinlogEvents(uint8_t *received_data, int *recvLen);
    int handleBufferBinlogEvents_test(vector<uint8_t> received_data, int *recvLen);
    int LengthEnodedBinlogInt(uint8_t *received_data);
    int LengthEnodedBinlogInt_test(vector<uint8_t> received_data);
    pair<int, vector<uint8_t> > skipLengthEnodedBinlogString(uint8_t *received_data, int pos, int *recvLen);
    pair<int, vector<uint8_t> > skipLengthEnodedBinlogString_test(vector<uint8_t> received_data, int pos, int *recvLen);
    pair<int, vector<uint8_t> > LengthEnodedBinlogString(uint8_t *received_data, int pos, int *recvLen);
    pair<int, vector<uint8_t> > LengthEnodedBinlogString_test(vector<uint8_t> received_data, int pos, int *recvLen);
    pair<int, vector<uint8_t> > takerBinlogData(int num, uint8_t *received_data, int pos, int *recvLen);
    pair<int, vector<uint8_t> > takerBinlogData_test(int num, vector<uint8_t> received_data, int pos, int *recvLen);
    pair<int, vector<uint8_t> > takerBinlogDataHeader(int num, uint8_t* received_data, int pos, int *recvLen, int header);
    pair<int, vector<uint8_t> > LengthEnodedBinlogHeaderString(uint8_t *received_data, int pos, int *recvLen, int header);
};

pair<int, vector<uint8_t> > tabkeBufferQueryEventHeader(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen);
pair<int, vector<uint8_t> > tabkeBufferQueryEventHeader_test(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int pos);


struct ComQueryResultRows {
    int rowsPacketLen[3];
    int rowsPacketNum;
    int rowsLen;
    vector<uint8_t> rowsContext;
    string rowsData;
};

struct ComQueryFieldPayload {
    int rowStart;
    int fieldsNum;
    int pos;
    uint8_t packetLen[3];
    uint8_t packetNum;
    uint8_t catalogLen;
    string catalog;
    uint8_t schemaLen;
    string schema;
    uint8_t tableNameLen;
    string tableName;
    uint8_t physicalTableLen;
    string physicalTable;
    uint8_t ColumnNameLen;
    string ColumnName;
    uint8_t physicalColumnNameLen;
    string physicalColumnName;
    uint8_t fixedLen;
    uint8_t characterSet[2];
    uint8_t columnLength[4];
    uint8_t type[1];
    uint8_t flags[2];
    uint8_t decimals[1];
    struct ComQueryResultRows ResultRows;
};

struct ComQueryField {
    int packetLen;
    int packetNum;
    int fieldsNum;
};


struct ComQueryEventPayload {
    vector<uint8_t> statusVars;
    vector<uint8_t> schemaName;
    vector<uint8_t> query;
    vector<uint8_t> checkSum;
};

struct ComQueryEventHeader {
    uint8_t slaveProxyID[4];
    uint8_t executionTime[4];
    uint8_t schemaLength[1];
    uint8_t errorCode[2];
    // for binlog-version >= 4
    uint8_t statusVarsLength[2];
    struct ComQueryEventPayload queryPayload;
};

struct ComRotateEventPayload {
    vector<uint8_t> eventContext;
    vector<uint8_t> checkSum;
};

struct ComRotateEventHeader {
    uint8_t position[8]; // for rotate event
    struct ComRotateEventPayload rotatePayload;
};


struct ComFormatDescPayload {
    uint8_t binlogVersion[2];
    vector<uint8_t> mysqlServerVersion;
    uint8_t createTimestamp[4];
    uint8_t eventHeaderLength;
    vector<uint8_t> eventTypeHeaderLengths; // string<EOF>
};

// ANONYMOUS_GTID_LOG_EVENT
struct ComAnonymousGITDLogPayload {
    uint8_t checkSum[4];
};
struct ComAnonymousGITDLogHeader {
    // 记录binlog格式，如果gtid_flags值为1，表示binlog中可能有以statement方式记录的binlog，如果为0表示，binlog中只有以row格式记录的binlog
    uint8_t gtidFlags;
    // 记录GTID中uuid的部分（不记录‘-’），每1个字节表示uuid中2个字符
    uint8_t sid[16];
    uint8_t gno[8];
    uint8_t logicalTimestampTypecode;
    uint8_t lastCommit[8];
    uint8_t sequenceNO[8];
    struct ComAnonymousGITDLogPayload anonymousGITDLogPayload;
};

struct ComTableMapPayloadFieldTypeMetaData {
    uint8_t columnNum;
    uint8_t columnType;
    vector<uint8_t> columnMetaData;
    uint8_t isNull;
};



struct ComTableMapPayload {
    uint8_t tableID[6];
    uint8_t flags[2];
    uint8_t schemaNameLength;
    string schemaName;
    uint8_t tableNameLength;
    string tableName;
    uint8_t fieldCound;
    vector<uint8_t> fieldType;
    vector<uint8_t> fieldSeq;
    uint8_t metaDataLength;
    vector<uint8_t> metaData;
//    vector<struct ComTableMapPayloadFieldTypeMetaData> metaData;
    vector<uint8_t> fieldIsNull;
    vector<uint8_t> checkSum;
};

struct ComTableMapHeader {
    // https://mariadb.com/kb/en/table_map_event/
    struct ComTableMapPayload tableMapPayload;
};

struct ComDeleteRowsPayload {

};

struct ComDeleteRowsHeader {

    struct ComDeleteRowsPayload DeleteRowsPayload;
};



struct ComUpdateRowsPayload {

};
struct ComUpdateRowsHeader {

    struct ComUpdateRowsPayload UpdateRowsPayload;
};

struct ComWriteRowsPayloadContext {
    int fieldNumber; // 第几列
    vector<uint8_t> fieldRows;  // 列内容
};

struct ComWriteRowsPayload {
    uint8_t fieldNumber; // 第几列
    uint8_t fieldType;
    uint8_t fieldTypeRealLength;  // 列类型编号对应的类型长度
    vector<uint8_t> fieldTypeMeta; // 列属性
    map<int, vector<uint8_t> > fieldContext; // 列内容 格式： key: 第几列 value: 列内容  做个链表吧
    struct ComWriteRowsPayloadContext WriteRowsPayloadContext;
    vector<uint8_t> checkSum;
};
struct ComWriteRowsHeader {
    struct ComWriteRowsPayload WriteRowsPayload;
};

struct ComRowsEventPayload {

};

struct ComRowsEventHeader {
    uint8_t tableID[6];
    uint64_t tableIDNum;
    uint8_t flags[2];
    vector<uint8_t> extraRowInfo;
//    uint8_t extraDataLength[2];
//    vector<uint8_t> extraData;
    uint8_t columnNumber; // number of columns in the table
    vector<uint8_t> columnsRepresentBitMap; // n = (number_of_columns + 7)/8
    // for _COMPRESSED_EVENT_V1
    uint8_t compressedHeader; // algorithm: (header & 0x07) >> 4 (always 0=zlib)   header_size: header & 0x07
    uint8_t headerSize = compressedHeader & 0x07;
    vector<uint8_t> unCompressedLength; // header_size
    vector<uint8_t> NULLBitMap;  // n = (number_of_columns + 7)/8
    uint8_t columnDataLength;
    string columnData;
    uint8_t checkSum[4];



    struct ComRowsEventPayload RowsEventPayload; // 备用
    struct ComWriteRowsHeader WriteRowsHeader;  // 记录write rows事件的内容
    struct ComUpdateRowsHeader UpdateRowsHeader;
    struct ComDeleteRowsHeader DeleteRowsHeader;



};

struct ComXIDPayload {
    // 8 byte
    uint8_t XIDTranscationNumber[8];
    vector<uint8_t> checkSum;
};

struct ComXIDHeader {
    struct ComXIDPayload XIDPayload;
};


struct ComBinlogDumpFields {
    uint8_t packetLen[3];
    uint8_t packetNum;
    uint8_t packetStat;
    uint8_t status;
    uint8_t timestamp[4];
    uint32_t eventType;
    uint8_t serverID[4];
    uint8_t eventSize[4];
    uint8_t logPos[4];
    uint8_t flags[2];
    uint8_t position[8]; // for rotate event
    vector<uint8_t> eventContext;
    uint8_t checkSum[4];
    // query event
    struct ComQueryEventHeader queryHeader;

    // rotate event
    struct ComRotateEventHeader rotateHeader;

    // format desc
    struct ComFormatDescPayload formatDesc;

    // ANONYMOUS_GTID_LOG_EVENT
    struct ComAnonymousGITDLogHeader anonymousGITDLogHeader;

    // table map event
    struct ComTableMapHeader TableMapHeader;

    // rows event:
    // 这里存放了update/delete/insert的列内容及相关的元数据信息
    struct ComRowsEventHeader RowsEventHeader;
    // write rows event
//    struct ComWriteRowsHeader WriteRowsHeader;


    // xid
    struct ComXIDHeader XIDHeader;
};





#endif //MYSQL_REPLICATER_PARSER_H
