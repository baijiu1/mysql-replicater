//
// Created by 白杰 on 2024/4/22.
//

#ifndef MYSQL_REPLICATER_BINLOGEVENT_H
#define MYSQL_REPLICATER_BINLOGEVENT_H

#include <cstdio>
#include <string>

using namespace std;
#define  MYSQL_TYPE_DECIMAL   0
#define  MYSQL_TYPE_TINY   1
#define  MYSQL_TYPE_SHORT   2
#define  MYSQL_TYPE_LONG   3
#define  MYSQL_TYPE_FLOAT   4
#define  MYSQL_TYPE_DOUBLE   5
#define  MYSQL_TYPE_NULL   6
#define  MYSQL_TYPE_TIMESTAMP   7
#define  MYSQL_TYPE_LONGLONG   8
#define  MYSQL_TYPE_INT24   9
#define  MYSQL_TYPE_DATE   10
#define  MYSQL_TYPE_TIME   11
#define  MYSQL_TYPE_DATETIME   12
#define  MYSQL_TYPE_YEAR   13
#define  MYSQL_TYPE_NEWDATE   14
#define  MYSQL_TYPE_VARCHAR   15
#define  MYSQL_TYPE_BIT   16
#define  MYSQL_TYPE_TIMESTAMP2  17
#define  MYSQL_TYPE_DATETIME2  18
#define  MYSQL_TYPE_TIME2  19
#define  MYSQL_TYPE_TYPED_ARRAY 20
#define  MYSQL_TYPE_JSON     245
#define  MYSQL_TYPE_NEWDECIMAL   246
#define  MYSQL_TYPE_ENUM   247
#define  MYSQL_TYPE_SET   248
#define  MYSQL_TYPE_TINY_BLOB   249
#define  MYSQL_TYPE_MEDIUM_BLOB   250
#define  MYSQL_TYPE_LONG_BLOB   251
#define  MYSQL_TYPE_BLOB   252
#define  MYSQL_TYPE_VAR_STRING   253
#define  MYSQL_TYPE_STRING   254
#define  MYSQL_TYPE_GEOMETRY   255


struct table_map_event_field_info {
    uint32_t table_id;
    uint8_t file_seq_number; // 标识第几列
    uint8_t field_type;
    string field_event_type;
    vector<int> field_meta_data;
};

pair<string, vector<uint8_t> > getEventTypeAndMetaData(uint8_t *metaData, uint8_t fieldType, int seq, int *metaDLen);
pair<int, vector<uint8_t> > takerBufferEventTypeAndMetaData(vector<uint8_t> metaD, uint8_t fieldType, int seq, int recvLen, int *metaDLen);

string int2b(uint8_t a);
class Event {
public:
    int Init(struct ComBinlogDumpFields *binlogFields);
    int parseEvent(uint8_t eventType, uint8_t* received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int pckNum);

    // 返回元数据属性
//    int parserMetaData(uint8_t eventType, uint8_t* received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int pckNum);

    // generation
    pair<int, vector<uint8_t> > tabkeBufferEventHeader(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen);

    // query event
    int readQueryEventContext(uint8_t* received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum);
    pair<int, vector<uint8_t> > tabkeBufferQueryEventHeader(uint8_t* received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen);


    // rotate event
    int readRotateEventContext(uint8_t* received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum);
    pair<int, vector<uint8_t> > takerBufferEOFPck(uint8_t* received_data, struct ComBinlogDumpFields *binlogFields, int pckEventLen, int *recvLen);
    pair<int, vector<uint8_t> > takerBufferEOFCheckSum(uint8_t* received_data, struct ComBinlogDumpFields *binlogFields, int pckEventLen, int *recvLen);
    pair<int, vector<uint8_t> > tabkeBufferRotateEventHeader(uint8_t* received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen);

    // format desc
    int readFormatDescEventContext(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum);

    // Anonymous GTID Log
    int readAnonymousGTIDLogEventContext(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum);

    // table map event
    int readTableMapEventContext(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum);
    int GetColumnMetaArray(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen);
    pair<int, vector<uint8_t> > LengthEnodedBinlogString(int *recvSize, uint8_t *data);
    int LengthEnodedBinlogInt(uint8_t *received_data);
    pair<int, vector<uint8_t> > takerBinlogData(int num, int *recvSize, uint8_t *data);
    int bitmapByteSize(int columnCount);

    // rows event
    // write rows event
    int readWriteRowsEventContext(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum);
    int hexToBinaryAndCount(string binary);



    // xid
    int readXIDEventContext(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum);

    int setFieldTypesAndMetaDataInfo(int tableID, vector<uint8_t> fieldType, vector<uint8_t> fieldSeq,
                                     uint8_t *metaData, int *metaDLen);

};

enum Log_event_type {
    /**
      Every time you add a type, you have to
      - Assign it a number explicitly. Otherwise it will cause trouble
        if a event type before is deprecated and removed directly from
        the enum.
      - Fix Format_description_event::Format_description_event().
    */
    UNKNOWN_EVENT = 0,
    /*
      Deprecated since mysql 8.0.2. It is just a placeholder,
      should not be used anywhere else.
    */
    START_EVENT_V3 = 1,
    QUERY_EVENT = 2,
    STOP_EVENT = 3,
    ROTATE_EVENT = 4,
    INTVAR_EVENT = 5,

    SLAVE_EVENT = 7,

    APPEND_BLOCK_EVENT = 9,
    DELETE_FILE_EVENT = 11,

    RAND_EVENT = 13,
    USER_VAR_EVENT = 14,
    FORMAT_DESCRIPTION_EVENT = 15,
    XID_EVENT = 16,
    BEGIN_LOAD_QUERY_EVENT = 17,
    EXECUTE_LOAD_QUERY_EVENT = 18,

    TABLE_MAP_EVENT = 19,

    /**
      The V1 event numbers are used from 5.1.16 until mysql-5.6.
    */
    WRITE_ROWS_EVENT_V1 = 23,
    UPDATE_ROWS_EVENT_V1 = 24,
    DELETE_ROWS_EVENT_V1 = 25,

    /**
      Something out of the ordinary happened on the master
     */
    INCIDENT_EVENT = 26,

    /**
      Heartbeat event to be send by master at its idle time
      to ensure master's online status to slave
    */
    HEARTBEAT_LOG_EVENT = 27,

    /**
      In some situations, it is necessary to send over ignorable
      data to the slave: data that a slave can handle in case there
      is code for handling it, but which can be ignored if it is not
      recognized.
    */
    IGNORABLE_LOG_EVENT = 28,
    ROWS_QUERY_LOG_EVENT = 29,

    /** Version 2 of the Row events */
    WRITE_ROWS_EVENT = 30,
    UPDATE_ROWS_EVENT = 31,
    DELETE_ROWS_EVENT = 32,

    GTID_LOG_EVENT = 33,
    ANONYMOUS_GTID_LOG_EVENT = 34,

    PREVIOUS_GTIDS_LOG_EVENT = 35,

    TRANSACTION_CONTEXT_EVENT = 36,

    VIEW_CHANGE_EVENT = 37,

    /* Prepared XA transaction terminal event similar to Xid */
    XA_PREPARE_LOG_EVENT = 38,

    /**
      Extension of UPDATE_ROWS_EVENT, allowing partial values according
      to binlog_row_value_options.
    */
    PARTIAL_UPDATE_ROWS_EVENT = 39,

    TRANSACTION_PAYLOAD_EVENT = 40,

    HEARTBEAT_LOG_EVENT_V2 = 41,
    /**
      Add new events here - right above this comment!
      Existing events (except ENUM_END_EVENT) should never change their numbers
    */
    ENUM_END_EVENT /* end marker */
};

//MYSQL_TYPE_DECIMAL byte = iota
//MYSQL_TYPE_TINY
//        MYSQL_TYPE_SHORT
//MYSQL_TYPE_LONG
//        MYSQL_TYPE_FLOAT
//MYSQL_TYPE_DOUBLE
//        MYSQL_TYPE_NULL
//MYSQL_TYPE_TIMESTAMP
//        MYSQL_TYPE_LONGLONG
//MYSQL_TYPE_INT24
//        MYSQL_TYPE_DATE
//MYSQL_TYPE_TIME
//        MYSQL_TYPE_DATETIME
//MYSQL_TYPE_YEAR
//        MYSQL_TYPE_NEWDATE
//MYSQL_TYPE_VARCHAR
//        MYSQL_TYPE_BIT
// 字段类型
//enum enum_field_types {
//    MYSQL_TYPE_DECIMAL=0, MYSQL_TYPE_TINY=1,
//    MYSQL_TYPE_SHORT=2,  MYSQL_TYPE_LONG=3,
//    MYSQL_TYPE_FLOAT=4,  MYSQL_TYPE_DOUBLE=5,
//    MYSQL_TYPE_NULL=6,   MYSQL_TYPE_TIMESTAMP=7,
//    MYSQL_TYPE_LONGLONG=8,MYSQL_TYPE_INT24=9,
//    MYSQL_TYPE_DATE=10,   MYSQL_TYPE_TIME=11,
//    MYSQL_TYPE_DATETIME=12, MYSQL_TYPE_YEAR=13,
//    MYSQL_TYPE_NEWDATE=14, MYSQL_TYPE_VARCHAR=15,
//    MYSQL_TYPE_BIT=16,
//    MYSQL_TYPE_TIMESTAMP2,
//    MYSQL_TYPE_DATETIME2 = 18,
//    MYSQL_TYPE_TIME2,
//    MYSQL_TYPE_JSON=245,
//    MYSQL_TYPE_NEWDECIMAL=246,
//    MYSQL_TYPE_ENUM=247,
//    MYSQL_TYPE_SET=248,
//    MYSQL_TYPE_TINY_BLOB=249,
//    MYSQL_TYPE_MEDIUM_BLOB=250,
//    MYSQL_TYPE_LONG_BLOB=251,
//    MYSQL_TYPE_BLOB=252,
//    MYSQL_TYPE_VAR_STRING=253,
//    MYSQL_TYPE_STRING=254,
//    MYSQL_TYPE_GEOMETRY=255
//};


#endif //MYSQL_REPLICATER_BINLOGEVENT_H
