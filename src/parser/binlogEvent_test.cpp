//
// Created by 白杰 on 2024/4/22.
//

#include "binlogEvent_test.h"
#include "../config/config.h"
#include <map>
#include "parser.h"
#include <iostream>
#include "cache.h"
#include <cstdint>
#include <vector>
#include "binlogTypes_test.h"
#include <algorithm>

using namespace std;

extern std::map<Log_event_type, int> BinlogEventMap;
extern std::map<int, Log_event_type> BinlogEventIntMap;
//map<uint32_t, struct ComBinlogDumpFields *> tableMapAndWriteRowsEvent;// table id : table map event
//std::map<uint32_t, ComBinlogDumpFields*>* WriteRowsEventMap = new std::map<uint32_t, ComBinlogDumpFields*>();
extern map<int, MyFunctionType> funcMap;
class Cache *cache;
map<uint64_t , vector<struct table_map_event_field_info> > field_info;
map<int, struct ComBinlogDumpFields*> tableMapAndWriteRowsEvent;
extern vector<struct ComBinlogDumpFields *> PayloadBinlogs;



int Event_test::Init(struct ComBinlogDumpFields *binlogFields) {
    // initialize
    memset(binlogFields, 0, sizeof(struct ComBinlogDumpFields) * 2);
    // write rows event initialize
    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldType = 0;
    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldNumber = 0;
    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.checkSum.clear();
    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.clear();
    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldTypeRealLength = 0;
    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldTypeMeta.clear();
    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.WriteRowsPayloadContext.fieldNumber = 0;
    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.WriteRowsPayloadContext.fieldRows.clear();

    // rotate event initialize
    std::fill(binlogFields->rotateHeader.position, binlogFields->rotateHeader.position + 8, 0);
    fill(binlogFields->rotateHeader.rotatePayload.eventContext.begin(), binlogFields->rotateHeader.rotatePayload.eventContext.end(), 0);
    binlogFields->rotateHeader.rotatePayload.checkSum.clear();

    // xid
    binlogFields->XIDHeader.XIDPayload.checkSum.clear();
    fill(binlogFields->XIDHeader.XIDPayload.XIDTranscationNumber, binlogFields->XIDHeader.XIDPayload.XIDTranscationNumber + 8, 0);


}

int Event_test::parseEvent(unsigned char eventType, vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int pckNum, int pos) {
    // 涉及事物的类型
//    QUERY_EVENT
//            TABLE_MAP_EVENT
//    WRITE_ROWS_EVENT_V2
//            UPDATE_ROWS_EVENT_V2
//    DELETE_ROWS_EVENT_V2

    Log_event_type Log_eventType = BinlogEventIntMap[eventType];
    int eventLen;
    switch (Log_eventType) {
        case UNKNOWN_EVENT:
            break;
        case START_EVENT_V3:
            break;
        case QUERY_EVENT:
            // pckNum： event 包的总长度
            // recvLen： 当前recv到的包的长度
            // eventLen： 当前event的header的长度
            // binlogFields： 存储相关信息的结构体指针
            // received_data： 从服务端接收到的binlog消息
            eventLen = BinlogEventMap[Log_eventType];
            readQueryEventContext(received_data, binlogFields, eventLen, recvLen, pckNum, pos);
            break;
        case STOP_EVENT:
            break;
        case ROTATE_EVENT:
            // 取到header长度，从flags后开始，也就是从20开始向后取header长度+4(checksum)
            eventLen = BinlogEventMap[Log_eventType];
            readRotateEventContext(received_data, binlogFields, eventLen, recvLen, pckNum, pos);
            break;
        case INTVAR_EVENT:
            break;
        case SLAVE_EVENT:
            break;
        case APPEND_BLOCK_EVENT:
            break;
        case DELETE_FILE_EVENT:
            break;
        case RAND_EVENT:
            break;
        case USER_VAR_EVENT:
            break;
        case FORMAT_DESCRIPTION_EVENT:
            // format desc
            // 84 length header
            eventLen = BinlogEventMap[Log_eventType];
            readFormatDescEventContext(received_data, binlogFields, eventLen, recvLen, pckNum, pos);
            break;
        case XID_EVENT:
            eventLen = 0;
            readXIDEventContext(received_data, binlogFields, eventLen, recvLen, pckNum, pos);
            break;
        case BEGIN_LOAD_QUERY_EVENT:
            break;
        case EXECUTE_LOAD_QUERY_EVENT:
            break;
        case TABLE_MAP_EVENT:
            eventLen = 0;
            readTableMapEventContext(received_data, binlogFields, eventLen, recvLen, pckNum, pos);
            break;
        case WRITE_ROWS_EVENT_V1:
            // from 5.1.16 until mysql-5.6.
            break;
        case UPDATE_ROWS_EVENT_V1:
            break;
        case DELETE_ROWS_EVENT_V1:
            break;
        case INCIDENT_EVENT:
            break;
        case HEARTBEAT_LOG_EVENT:
            break;
        case IGNORABLE_LOG_EVENT:
            break;
        case ROWS_QUERY_LOG_EVENT:
            break;
        case UPDATE_ROWS_EVENT:
        case DELETE_ROWS_EVENT:
        case WRITE_ROWS_EVENT:
            eventLen = BinlogEventMap[Log_eventType];
            readWriteRowsEventContext(received_data, binlogFields, eventLen, recvLen, pckNum, pos);
            break;
        case GTID_LOG_EVENT:
            break;
        case ANONYMOUS_GTID_LOG_EVENT:
            eventLen = 0;
            readAnonymousGTIDLogEventContext(received_data, binlogFields, eventLen, recvLen, pckNum, pos);
            break;
        case PREVIOUS_GTIDS_LOG_EVENT:
            break;
        case TRANSACTION_CONTEXT_EVENT:
            break;
        case VIEW_CHANGE_EVENT:
            break;
        case XA_PREPARE_LOG_EVENT:
            break;
        case PARTIAL_UPDATE_ROWS_EVENT:
            break;
        case TRANSACTION_PAYLOAD_EVENT:
            break;
        case HEARTBEAT_LOG_EVENT_V2:
            break;
        case ENUM_END_EVENT:
            break;
    }
}


int Event_test::readXIDEventContext(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum, int pos) {
    printf("\n----------------------------XID---------------------------\n");
    if (eventLen < 0) {
        printf("XID event header length error!exit...");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue;
    // xid transcation number
//    printf("\n recvLen1: %d \n", *recvLen);
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 8, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->XIDHeader.XIDPayload.XIDTranscationNumber[i] = outValue.second[i];
//        printf(" %#x ", outValue.second[i]);
    }
    pos += outValue.first;
    // check sum
//    printf("\ncheck sum\n");
//    printf("\n recvLen2: %d \n", *recvLen);
//    *recvLen -= 1;
    tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4, pos);
//    printf("\n recvLen3: %d \n", *recvLen);
//    printf("\n check sum data\n");
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->XIDHeader.XIDPayload.checkSum.push_back(outValue.second[i]);
//        printf(" %#x ", outValue.second[i]);
    }
    printf("\n----------------------------XID---------------------------");
    return 0;
}




int Event_test::readWriteRowsEventContext(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum, int pos) {
    printf("\n read write rows event! \n");
    if (eventLen < 0) {
        printf("wirte rows event header length error!exit...");
        exit(1);
    }
    printf("\n");
    for (int i = 0; i < *recvLen; ++i) {
//        printf(" %#x ", received_data[i]);
    }
    pair<int, vector<uint8_t> > outValue;
    // table id
//    int table_id = 0;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 6, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->RowsEventHeader.tableID[i] = outValue.second[i];
//        table_id += binlogFields->RowsEventHeader.tableID[i];
    }
    pos += outValue.first;
    swap(binlogFields->RowsEventHeader.tableID[0], binlogFields->RowsEventHeader.tableID[5]);
    swap(binlogFields->RowsEventHeader.tableID[1], binlogFields->RowsEventHeader.tableID[4]);
    swap(binlogFields->RowsEventHeader.tableID[2], binlogFields->RowsEventHeader.tableID[3]);
    uint64_t table_id;
    for (int i = 6; i > 0; --i) {
        if (i == 0) {
            break;
        }
        table_id = (outValue.second[i] << i * 8) | outValue.second[0];
    }
    binlogFields->RowsEventHeader.tableIDNum = table_id;
    // flags
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 2, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->RowsEventHeader.flags[i] = outValue.second[i];
    }
    pos += outValue.first;
    // extra rows info
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 2, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->RowsEventHeader.extraRowInfo.push_back(outValue.second[i]);
    }
    pos += outValue.first;
    // width as number of column
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 1, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->RowsEventHeader.columnNumber = outValue.second[i];
    }
    pos += outValue.first;
    // cols bitmap length: (num of columns+7)/8
    int oneCount = 0;
    string binaryStr;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, bitmapByteSize(binlogFields->RowsEventHeader.columnNumber), pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->RowsEventHeader.columnsRepresentBitMap.push_back(outValue.second[i]);
        binaryStr = int2b(binlogFields->RowsEventHeader.columnsRepresentBitMap[i]);
        oneCount += hexToBinaryAndCount(binaryStr);
    }
    pos += outValue.first;
//    if UPDATE_ROWS_EVENTv1 or v2 {
//                string.var_len     columns-present-bitmap2, length: (num of columns+7)/8
//        }

    // nul-bitmap, length (bits set in 'columns-present-bitmap1'+7)/8
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, bitmapByteSize(oneCount), pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->RowsEventHeader.NULLBitMap.push_back(outValue.second[i]);
    }
    pos += outValue.first;
    if (field_info.count(table_id) == 0) {
        printf("table id not equality. exit...");
        exit(1);
    }

    // 知道第几个列为NULL，就跳过   1 为NULL  小端序
    uint64_t littleEndian[outValue.second.size()];
    for (size_t i = 0; i < outValue.second.size(); ++i) {
        littleEndian[i] = binlogFields->RowsEventHeader.NULLBitMap[outValue.second.size() - 1 - i];
    }
    string binaryString;
    for (const auto& hexValue : littleEndian) {
        std::bitset<8> binary(hexValue);
        binaryString += binary.to_string();
    }
    reverse(binaryString.begin(), binaryString.end());

    vector<uint8_t> fieldData;
    for (int i = 0; i < field_info[table_id].size(); ++i) {
        // isUnsigned = f[i].Flag&UNSIGNED_FLAG > 0  暂时先不判断Unsigned，太复杂了
        printf("\n -----------field types: %#x -----------\n", field_info[table_id][i].field_type);
        auto it = funcMap.find(field_info[table_id][i].field_type);
        if (it != funcMap.end()) {
            if (binaryString[i] == '1') {
                // 插入值为NULL，调用MYSQL_TYPE_NULL函数
                funcMap[MYSQL_TYPE_NULL](received_data, binlogFields, recvLen, 1, i, pos);
                continue;
            }
            outValue = it->second(received_data, binlogFields, recvLen, 1, i, pos); // 调用找到的函数
            pos += outValue.first;
        } else {
            printf(" type: %#x ", field_info[table_id][i].field_type);
            std::cout << "Invalid type" << std::endl;
        }
    }
    pos += outValue.first;
    // check sum
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.checkSum.push_back(outValue.second[i]);
    }
    printf("\n read write rows event end! \n");
    return 0;
//    if UPDATE_ROWS_EVENTv1 or v2 {
//                string.var_len       nul-bitmap, length (bits set in 'columns-present-bitmap2'+7)/8
//                string.var_len       value of each field as defined in table-map
//        }
};

string int2b(uint8_t a)
{
    string str;
    if(a == 0)
    {
        str ="0";
    }
    else
    {
        while(a != 0)
        {
            if((a&0x01) == 0)
            {
                str.insert(0, "0");
            }
            else
            {
                str.insert(0, "1");
            }
            a = a>>1;
        }
    }
    return str;
}

int Event_test::hexToBinaryAndCount(string binary) {
    // 计算1的个数
    int count = 0;
    for (int i = 0; i < binary.size(); ++i) {
        if (binary[i] == '1') {
            count += 1;
        }
    }
    return count;
}


int Event_test::readTableMapEventContext(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum, int pos) {
    if (eventLen < 0) {
        printf("Anonymous GTID Log event header length error!exit...");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue;
//    uint32_t table_id = 0;
    // table id
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 6, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->TableMapHeader.tableMapPayload.tableID[i] = outValue.second[i];
//        table_id += binlogFields->TableMapHeader.tableMapPayload.tableID[i];
    }
    swap(binlogFields->TableMapHeader.tableMapPayload.tableID[0], binlogFields->TableMapHeader.tableMapPayload.tableID[5]);
    swap(binlogFields->TableMapHeader.tableMapPayload.tableID[1], binlogFields->TableMapHeader.tableMapPayload.tableID[4]);
    swap(binlogFields->TableMapHeader.tableMapPayload.tableID[2], binlogFields->TableMapHeader.tableMapPayload.tableID[3]);
    uint64_t table_id;
    for (int i = 6; i > 0; --i) {
        if (i == 0) {
            break;
        }
        table_id = (outValue.second[i] << i * 8) | outValue.second[0];
    }
    // pos: 24
    pos += outValue.first;
    // pos: 30
//    table_id = result;
    //    printf("\n table_id: %d \n", table_id);
    // flags
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 2, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->TableMapHeader.tableMapPayload.flags[i] = outValue.second[i];
    }
    // schema name length
    pos += outValue.first;
    // pos: 32
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 1, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->TableMapHeader.tableMapPayload.schemaNameLength = outValue.second[i];
    }
    // schema name
    pos += outValue.first;

    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, binlogFields->TableMapHeader.tableMapPayload.schemaNameLength + 1, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->TableMapHeader.tableMapPayload.schemaName.push_back(outValue.second[i]);
    }
    // table name length
    pos += outValue.first;

    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 1, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->TableMapHeader.tableMapPayload.tableNameLength = outValue.second[i];
    }
    // table name
    pos += outValue.first;
//    printf("\n table map --> table name! \n");
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, binlogFields->TableMapHeader.tableMapPayload.tableNameLength + 1, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->TableMapHeader.tableMapPayload.tableName.push_back(outValue.second[i]);
//        printf(" %c ", binlogFields->TableMapHeader.tableMapPayload.tableName[i]);
    }
    printf("%s.%s", binlogFields->TableMapHeader.tableMapPayload.schemaName.c_str(), binlogFields->TableMapHeader.tableMapPayload.tableName.c_str());
    // field count string<len>类型
    //
//    outValue = LengthEnodedBinlogString(recvLen, received_data);
    pos += outValue.first;

    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 1, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->TableMapHeader.tableMapPayload.fieldCound = outValue.second[i];
    }
//    printf("\n field count: %d ", binlogFields->TableMapHeader.tableMapPayload.fieldCound);
    // field type 占field count位
    // count 05  ->  03 0f 08 fe fc
//    outValue = LengthEnodedBinlogString(recvLen, received_data);
    pos += outValue.first;

    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, binlogFields->TableMapHeader.tableMapPayload.fieldCound, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->TableMapHeader.tableMapPayload.fieldType.push_back(outValue.second[i]);
        // 把列的序号也存储进去，知道是第几列
        binlogFields->TableMapHeader.tableMapPayload.fieldSeq.push_back(i);
    }
    // meta data
    // string<len>
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 1, pos);
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, outValue.second[0], pos);
//    outValue = LengthEnodedBinlogString(recvLen, received_data, pos);
//    printf("\n meta length: %d \n", outValue.first);
    uint8_t metaD[outValue.second.size()];
    for (int i = 0; i < outValue.second.size(); ++i) {
        // 这里需要结合field type来判断是第几列，有meta data
        // 循环type -> 根据类型判断存不存在meta data，如果不存在就跳过，如果存在就记录当时的i，也就是第几列，值就是meta data
        // 用第几列来把类型和meta data关联起来。一个map<int, vector<struct table_map_event_field_info> >
        // 一个key: table_id  value（vector<struct table_map_event_field_info>）: i, type，event_type， meta_data
        binlogFields->TableMapHeader.tableMapPayload.metaData.push_back(outValue.second[i]);
        metaD[i] = outValue.second[i];
//        printf(" \n mt: %#x \n", metaD[i]);
    }
//    pos += outValue.first + 1;
    // meta 总长度
    int metaLen[1] = {static_cast<int>(outValue.second.size())};
    int * metaDLen = metaLen;


    // field is null (field_count + 7) / 8
    int nullBitmapSize = bitmapByteSize(binlogFields->TableMapHeader.tableMapPayload.fieldCound);
//    printf(" \nnullBitmapSize: %d \n", nullBitmapSize);
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, nullBitmapSize, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->TableMapHeader.tableMapPayload.fieldIsNull.push_back(outValue.second[i]);
    }

    // @param
    // vector: binlogFields->TableMapHeader.tableMapPayload.metaData
    // vector: binlogFields->TableMapHeader.tableMapPayload.fieldSeq
    // vector: binlogFields->TableMapHeader.tableMapPayload.fieldType
    // table_id: int
    // 应为67
    setFieldTypesAndMetaDataInfo(table_id, binlogFields->TableMapHeader.tableMapPayload.fieldType,
                                 binlogFields->TableMapHeader.tableMapPayload.fieldSeq,
                                 binlogFields->TableMapHeader.tableMapPayload.metaData, metaDLen);

    pos += outValue.first;
    // check sum
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->TableMapHeader.tableMapPayload.checkSum.push_back(outValue.second[i]);
    }
    // decode decodeOptionalMeta


}

int Event_test::GetColumnMetaArray(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen) {
    // 这里未完成
    if (eventLen != -100001111) {
        printf("type->tableMapEvent error!");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue;
    // LONG: int
    int pos = 0;
    for (int i = 0; i < binlogFields->TableMapHeader.tableMapPayload.fieldType.size(); ++i) {
        struct ComTableMapPayloadFieldTypeMetaData fieldMeatData;
        // 记录元信息
        fieldMeatData.columnNum = i;
        fieldMeatData.columnType = binlogFields->TableMapHeader.tableMapPayload.fieldType[i];
//        case MYSQL_TYPE_STRING:
//            var x = uint16(data[pos]) << 8 // real type
//            x += uint16(data[pos+1])       // pack or field length
//        e.ColumnMeta[i] = x
//        pos += 2
//        case MYSQL_TYPE_NEWDECIMAL:
//            var x = uint16(data[pos]) << 8 // precision
//            x += uint16(data[pos+1])       // decimals
//        e.ColumnMeta[i] = x
//        pos += 2
//        case MYSQL_TYPE_VAR_STRING,
//        MYSQL_TYPE_VARCHAR,
//                MYSQL_TYPE_BIT:
//        e.ColumnMeta[i] = binary.LittleEndian.Uint16(data[pos:])
//        pos += 2
//        case MYSQL_TYPE_BLOB,
//        MYSQL_TYPE_DOUBLE,
//                MYSQL_TYPE_FLOAT,
//                MYSQL_TYPE_GEOMETRY,
//                MYSQL_TYPE_JSON:
//        e.ColumnMeta[i] = uint16(data[pos])
//        pos++
//        case MYSQL_TYPE_TIME2,
//        MYSQL_TYPE_DATETIME2,
//                MYSQL_TYPE_TIMESTAMP2:
//        e.ColumnMeta[i] = uint16(data[pos])
//        pos++
//        case MYSQL_TYPE_NEWDATE,
//        MYSQL_TYPE_ENUM,
//                MYSQL_TYPE_SET,
//                MYSQL_TYPE_TINY_BLOB,
//                MYSQL_TYPE_MEDIUM_BLOB,
//                MYSQL_TYPE_LONG_BLOB:
//        return errors.Errorf("unsupport type in binlog %d", t)
//        default:
//            e.ColumnMeta[i] = 0
        if (binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_STRING) {
            uint8_t metaD = binlogFields->TableMapHeader.tableMapPayload.metaData[pos] << 8;
            metaD += binlogFields->TableMapHeader.tableMapPayload.metaData[pos + 1];
            fieldMeatData.columnMetaData.push_back(metaD);
            pos += 2;
        } else if (binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_NEWDECIMAL) {
            uint8_t metaD = binlogFields->TableMapHeader.tableMapPayload.metaData[pos] << 8;
            metaD += binlogFields->TableMapHeader.tableMapPayload.metaData[pos + 1];
            fieldMeatData.columnMetaData.push_back(metaD);
            pos += 2;
        } else if (binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_VAR_STRING ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_VARCHAR ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_BIT) {
            uint8_t metaD = binlogFields->TableMapHeader.tableMapPayload.metaData[i] << 8;
            metaD += binlogFields->TableMapHeader.tableMapPayload.metaData[i + 1];
            fieldMeatData.columnMetaData.push_back(metaD);
            pos += 2;
        }

        if (binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_TINY ||
            binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_SHORT ||
            binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_INT24||
            binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_LONG||
            binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_LONGLONG||
            binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_YEAR||
            binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_DATE) {
            // metadata为空
//            vector<uint8_t> data;
//            data.push_back(00);
//            outValue = tabkeBufferEventHeader(received_data, binlogFields, recvLen, 0);
            fieldMeatData.columnMetaData.push_back(0);
            // 这里应该更新一下received_data 奇怪的是有的table map是没有显示的

//                try {
//                    binlogFields->TableMapHeader.tableMapPayload.metaData[binlogFields->TableMapHeader.tableMapPayload.fieldType[i]].push_back(0);
//                }catch (const std::exception& e) {
//                    printf("\n %s \n", e.what());
//                    std::cerr << "Exception: " << e.what() << std::endl;
//                }
//            binlogFields->TableMapHeader.tableMapPayload.metaData.insert(make_pair(binlogFields->TableMapHeader.tableMapPayload.fieldType[i], data));

        } else if (binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_FLOAT ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_DOUBLE ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_BLOB ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_JSON ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_GEOMETRY) {
            // 1 byte
            outValue = tabkeBufferEventHeader(received_data, binlogFields, recvLen, 1);
//            vector<uint8_t> data;
            for (int i = 0; i < outValue.second.size(); ++i) {
                fieldMeatData.columnMetaData.push_back(outValue.second[i]);
//                binlogFields->TableMapHeader.tableMapPayload.metaData[binlogFields->TableMapHeader.tableMapPayload.fieldType[i]].push_back(outValue.second[i]);
//                data.push_back(outValue.second[i]);
            }
//            binlogFields->TableMapHeader.tableMapPayload.metaData.insert(make_pair(binlogFields->TableMapHeader.tableMapPayload.fieldType[i], data));

        } else if (binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_NEWDECIMAL ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_VARCHAR ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_STRING ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_ENUM ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_SET ||
                   binlogFields->TableMapHeader.tableMapPayload.fieldType[i] == MYSQL_TYPE_BIT) {
            // 2 byte
            outValue = tabkeBufferEventHeader(received_data, binlogFields, recvLen, 2);
//            vector<uint8_t> data;
            for (int i = 0; i < outValue.second.size(); ++i) {
                fieldMeatData.columnMetaData.push_back(outValue.second[i]);
//                binlogFields->TableMapHeader.tableMapPayload.metaData[binlogFields->TableMapHeader.tableMapPayload.fieldType[i]].push_back(outValue.second[i]);
//                data.push_back(outValue.second[i]);
            }
//            binlogFields->TableMapHeader.tableMapPayload.metaData.insert(make_pair(binlogFields->TableMapHeader.tableMapPayload.fieldType[i], data));
        } else {
            return 1;
        }
//        binlogFields->TableMapHeader.tableMapPayload.metaData.push_back(fieldMeatData);
    }
}

int Event_test::readAnonymousGTIDLogEventContext(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum, int pos) {
    if (eventLen < 0) {
        printf("Anonymous GTID Log event header length error!exit...");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue;
    // gtid_flags
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 1, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->anonymousGITDLogHeader.gtidFlags = outValue.second[i];
    }
    pos += outValue.first;
    // sid
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 16, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->anonymousGITDLogHeader.sid[i] = outValue.second[i];
    }
    pos += outValue.first;
    // gno
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 8, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->anonymousGITDLogHeader.gno[i] = outValue.second[i];
    }
    pos += outValue.first;
    // logical_timestamp_typecode
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 1, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->anonymousGITDLogHeader.logicalTimestampTypecode = outValue.second[i];
    }
    pos += outValue.first;
    // last_commit
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 8, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->anonymousGITDLogHeader.lastCommit[i] = outValue.second[i];
    }
    pos += outValue.first;
    // sequence_no
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 8, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->anonymousGITDLogHeader.lastCommit[i] = outValue.second[i];
    }
    pos += outValue.first;
    // checksum
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->anonymousGITDLogHeader.anonymousGITDLogPayload.checkSum[i] = outValue.second[i];
        printf(" \n check sum: %#x \n", binlogFields->anonymousGITDLogHeader.anonymousGITDLogPayload.checkSum[i]);
    }

}

int Event_test::readFormatDescEventContext(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum, int pos) {
    if (eventLen < 0) {
        printf("format desc event header length error!exit...");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue;
    // binlog version
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 2, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->formatDesc.binlogVersion[i] = outValue.second[i];
    }
    pos += outValue.first;
    // mysql server version 50
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 50, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->formatDesc.mysqlServerVersion.push_back(outValue.second[i]);
    }
    pos += outValue.first;
    // create timestamp
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->formatDesc.createTimestamp[i] = outValue.second[i];
    }
    pos += outValue.first;
    // event-header-length always 19 -> 0x13
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 1, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->formatDesc.eventHeaderLength = outValue.second[i];
    }
    pos += outValue.first;
    // string<EOF>
    // 为什么要减 4？
    int formatLength = pckNum - (24 + 2 + 50 + 4 + 1 -4 );
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, formatLength, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->formatDesc.eventTypeHeaderLengths.push_back(outValue.second[i]);
    }
    // check sum
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
//        binlogFields->formatDesc.eventTypeHeaderLengths.push_back(outValue.second[i]);
    }
}

int Event_test::readQueryEventContext(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum, int pos) {
    // eventLen 13
    if (eventLen < 0) {
        printf("query event header length error!exit...");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue;
    // 4: slave_proxy_id
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->queryHeader.slaveProxyID[i] = outValue.second[i];
//        printf(" slaveProxyID: %#x ", binlogFields->queryHeader.slaveProxyID[i]);
    }
    pos += outValue.first;

    // 4: execution time
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->queryHeader.executionTime[i] = outValue.second[i];
    }
    pos += outValue.first;
    // 1: schema length
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 1, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->queryHeader.schemaLength[i] = outValue.second[i];
    }
    pos += outValue.first;
    // 2: error code
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 2, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->queryHeader.errorCode[i] = outValue.second[i];
    }
    pos += outValue.first;
    // 2: status_vars length
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 2, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->queryHeader.statusVarsLength[i] = outValue.second[i];
    }
    // 向后取长度为binlogFields->queryHeader.statusVarsLength[0]的
    int statusVarsLen;
    pos += outValue.first;
    if (binlogFields->queryHeader.statusVarsLength[1] == 0) {
        statusVarsLen = binlogFields->queryHeader.statusVarsLength[0];
        // 只有binlogFields->queryHeader.statusVarsLength[0]
        outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, statusVarsLen, pos);
        for (int i = 0; i < outValue.second.size(); ++i) {
            binlogFields->queryHeader.queryPayload.statusVars.push_back(outValue.second[i]);
        }
    } else {
        // 拼接0位和1位
        statusVarsLen = binlogFields->queryHeader.statusVarsLength[0] + binlogFields->queryHeader.statusVarsLength[1];
        outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, statusVarsLen, pos);
        for (int i = 0; i < outValue.second.size(); ++i) {
            binlogFields->queryHeader.queryPayload.statusVars.push_back(outValue.second[i]);
        }
    }
    pos += outValue.first;
    // schema name
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, binlogFields->queryHeader.schemaLength[0], pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->queryHeader.queryPayload.schemaName.push_back(outValue.second[i]);
    }
    // eventLen: 13 + 24: header fixed
    int queryLength = pckNum - (eventLen + 24 + statusVarsLen + binlogFields->queryHeader.schemaLength[0]);
//    printf(" \n queryLength: %d pckNum: %d \n", queryLength, pckNum);
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, queryLength, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->queryHeader.queryPayload.query.push_back(outValue.second[i]);
        printf(" %c ", outValue.second[i]);
    }

    printf(" \n ");
    pos += outValue.first;
    // 扫描check sum字段
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->queryHeader.statusVarsLength[i] = outValue.second[i];
    }
//    outValue = takerBufferEOFCheckSum(received_data, binlogFields, 4, recvLen);
//    for (int i = 0; i < outValue.second.size(); ++i) {
//        binlogFields->queryHeader.queryPayload.checkSum.push_back(outValue.second[i]);
////        printf(" %#x ", outValue.second[i]);
//    }
    return 0;
};

pair<int, vector<uint8_t> > Event_test::tabkeBufferQueryEventHeader(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int pos) {
    vector<uint8_t> contextData;
    if (eventLen >= 0) {
        for (int i = 0; i < eventLen; ++i) {
            contextData.push_back(received_data[i + pos]);
//            binlogFields->queryHeader.slaveProxyID[i] = received_data[i];
//            contextData[i] = data[i];
//            printf(" %#x ", binlogFields->position[i]);
        }
        return make_pair(eventLen, contextData);
    }
};



int Event_test::readRotateEventContext(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int eventLen, int *recvLen, int pckNum, int pos) {
    // 先把8个字节的ROTATE_EVENT header去掉
    // eventLen 8
    pair<int, vector<uint8_t> > outValue;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 8, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->queryHeader.slaveProxyID[i] = outValue.second[i];
//        printf(" slaveProxyID: %#x ", binlogFields->queryHeader.slaveProxyID[i]);
    }
    pos += outValue.first;
//    tabkeBufferRotateEventHeader(received_data, binlogFields, recvLen, eventLen);
    // string<EOF> 总长度减去其他确定的长度，就是内容长度
    // 这里把内容拿出来
//    printf(" pckNum : %d ", pckNum);
    // 最后一个4没想出怎么来的
//    int pckEventLen = pckNum - (5 + 4 + 1 + 4 + 4 + 4 + 2 + 4 + 4);
    int pckEventLen = pckNum - pos;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, pckEventLen, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->rotateHeader.rotatePayload.checkSum.push_back(outValue.second[i]);
//        printf("");
    }
    pos += outValue.first;
    printf("\n rotate event check sum\n");
    // 扫描check sum字段
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->rotateHeader.rotatePayload.checkSum.push_back(outValue.second[i]);
//        printf("");
    }
    return 0;
};

pair<int, vector<uint8_t> > Event_test::takerBufferEOFCheckSum(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int pckEventLen, int *recvLen) {
    vector<uint8_t> contextData;
    if (pckEventLen >= 0) {
        for (int i = 0; i < pckEventLen; ++i) {
//            contextData.push_back(received_data[i]);
            binlogFields->checkSum[i] = received_data[i];
//            contextData[i] = data[i];
//            printf(" %#x ", binlogFields->checkSum[i]);
        }
        // 减去num，更新data数据
        // 这里更新data到以下一个长度开头
        for (int i = 0; i < *recvLen; ++i) {
            // 这里
            received_data[i -1] = received_data[ i + pckEventLen - 1 ];
//            printf(" n %#x ", received_data[i - 1]);
        }
        *recvLen -= pckEventLen;
        return make_pair(pckEventLen, contextData);
    }
};

pair<int, vector<uint8_t> > Event_test::takerBufferEOFPck(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int pckEventLen, int *recvLen) {
    vector<uint8_t> contextData;
    if (pckEventLen >= 0) {
        for (int i = 0; i < pckEventLen; ++i) {
//            contextData.push_back(received_data[i]);
            binlogFields->eventContext.push_back(received_data[i]);
//            contextData[i] = data[i];
//            printf(" %#x ", binlogFields->eventContext[i]);
        }
        for (int i = 0; i < *recvLen; ++i) {
            // 这里
            received_data[i -1] = received_data[ i + pckEventLen - 1 ];
//            printf(" n %#x ", received_data[i - 1]);
        }
        *recvLen -= pckEventLen;
        return make_pair(pckEventLen, contextData);
    }
};

pair<int, vector<uint8_t> > Event_test::tabkeBufferRotateEventHeader(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen) {
    vector<uint8_t> contextData;
    if (eventLen >= 0) {
        for (int i = 0; i < eventLen; ++i) {
//            contextData.push_back(received_data[i]);
            binlogFields->position[i] = received_data[i];
//            contextData[i] = data[i];
//            printf(" %#x ", binlogFields->position[i]);
        }
        // 减去num，更新data数据
        // 这里更新data到以下一个长度开头
        for (int i = 0; i < *recvLen; ++i) {
            // 这里
            received_data[i -1] = received_data[ i + eventLen - 1 ];
//            printf(" n %#x ", received_data[i - 1]);
        }
        *recvLen -= eventLen;
        return make_pair(eventLen, contextData);
    }
};


pair<int, vector<uint8_t> > Event_test::tabkeBufferEventHeader(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen) {
    vector<uint8_t> contextData;
    if (eventLen >= 0) {
        for (int i = 0; i < eventLen; ++i) {
            contextData.push_back(received_data[i]);
//            binlogFields->queryHeader.slaveProxyID[i] = received_data[i];
//            contextData[i] = data[i];
//            printf(" %#x ", binlogFields->position[i]);
        }
        // 减去num，更新data数据
        // 这里更新data到以下一个长度开头
//        printf("\n");
        for (int i = 0; i < *recvLen; ++i) {
            // 这里
            received_data[i] = received_data[ i + eventLen ];
//            printf(" %#x ", received_data[i]);
        }
//        printf("\n");
        *recvLen -= eventLen;
        return make_pair(eventLen, contextData);
    }
};


// string<Len> LengthEncodedString
pair<int, vector<uint8_t> > Event_test::takerBinlogData(int num, int *recvSize, vector<uint8_t> data, int pos) {
    // num: 内容开始长度
    vector<uint8_t> contextData; // 具体的内容
    if (num >= 0) {
//        printf(" num: %d ", num);
        for (int i = 1; i <= num; ++i) {
            contextData.push_back(data[i + pos]);
        }
        // 减去num，更新data数据
        // 这里更新data到以下一个长度开头
//        for (int i = 1; i < recvSize[0]; ++i) {
//            data[i - 1] = data[ i + num  ];
//        }
//        recvSize[0] -= num;
        return make_pair(num, contextData);
    }
};
int Event_test::LengthEnodedBinlogInt(vector<uint8_t> received_data) {
    // 函数返回第一个字节的长度
    int n = 0, num = 0;
    switch (received_data[0]) {
        // NULL
        case 0xfb:
            n = 1;
            return 1;
        case 0xfc:
            n = 3;
            return 1;
        case 0xfd:
            n = 4;
            return 1;
        case 0xfe:
            n = 9;
            return 1;
    }
    num = received_data[0];
    return num;

}
pair<int, vector<uint8_t> > Event_test::LengthEnodedBinlogString(int *recvSize, vector<uint8_t> data, int pos) {
    // get length
    int num = LengthEnodedBinlogInt(data);
    printf(" num!: %#x ", num);
    if (num < 0) {
        printf("encode binlog event string num failed!");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue = takerBinlogData(num, recvSize, data, pos);
    return outValue;
}

int Event_test::bitmapByteSize(int columnCount) {
    return (columnCount + 7) / 8;
}

int Event_test::setFieldTypesAndMetaDataInfo(int tableID, vector<uint8_t> fieldType, vector<uint8_t> fieldSeq,
                                             vector<uint8_t> metaData, int *metaDLen) {
    field_info.clear();
    vector<struct table_map_event_field_info> fields;
    fields.clear();
//    struct table_map_event_field_info fieldInfo;
    pair<string , vector<uint16_t> > outValue;
    int metaPos = 0;
    for (int i = 0; i < fieldType.size(); ++i) {
        struct table_map_event_field_info* fieldInfo = (struct table_map_event_field_info*) malloc(sizeof(struct table_map_event_field_info) * 2);
        memset(fieldInfo, 0, sizeof(fieldInfo) * 2);
        //        printf("\n meta data len: %d \n", *metaDLen);
        if (*metaDLen == 0) {
            printf("\nmeta data 没有了\n");
//            break;
        }
        fieldInfo->table_id = tableID;
        fieldInfo->file_seq_number = i;
        fieldInfo->field_type = fieldType[i];
//        printf("\n %#x  %#x  %#x \n", metaData[0], metaData[1], metaData[2]);
        outValue = getEventTypeAndMetaData(metaData, fieldType[i], i, metaDLen, metaPos);
        if (outValue.second[0] != 65000) {
            metaPos += outValue.second.size();
        }
        fieldInfo->field_event_type = outValue.first;
        for (int j = 0; j < outValue.second.size(); ++j) {
            // set 类型248了
            fieldInfo->field_meta_data.push_back(outValue.second[j]);
        }
        // 拿到是否为NULL


        fields.push_back(*fieldInfo);
        free(fieldInfo);
    }
    field_info.insert(make_pair(tableID, fields));
//    field_info[tableID] = fields;
    return 0;
}


pair<int, vector<uint8_t> > takerBufferEventTypeAndMetaData(vector<uint8_t> metaD, uint8_t fieldType, int seq, int eventLen, int *metaDLen, int pos) {
    vector<uint8_t> contextData;
    if (eventLen >= 0) {
        for (int i = 0; i < eventLen; ++i) {
            contextData.push_back(metaD[i + pos]);
//            binlogFields->queryHeader.slaveProxyID[i] = received_data[i];
//            contextData[i] = data[i];
//            printf(" %#x ", binlogFields->position[i]);
        }
        // 这里更新data到以下一个长度开头
//        for (int i = 0; i < metaD.size(); ++i) {
//            // 这里
//            metaD[i] = metaD[ i + eventLen ];
////            printf(" n %#x ", metaD[i]);
//        }
        return make_pair(eventLen, contextData);
    }
};

pair<string, vector<uint16_t> > Event_test::getEventTypeAndMetaData(vector<uint8_t> metaData, uint8_t fieldType, int seq, int *metaDLen, int pos) {
    // 判断type，写入struct. 判断类型，看有多少个meta data字节
    //    Type	                                 Length	        Description
    //    MYSQL_TYPE_BLOB	                        1	        Number of bytes for length: e.g. 4 bytes means length is stored in a 4 byte integer)
    //    MYSQL_TYPE_DATETIME2	                    1	        Length of microseconds
    //    MYSQL_TYPE_DECIMAL	                    2	        Not in use anymore
    //    MYSQL_TYPE_DOUBLE ,MYSQL_TYPE_FLOAT	    1	        length (4 or 8 bytes
    //    MYSQL_TYPE_STRING	                        2	        1st byte contains type (MYSQL_TYPE_STRING, MYSQL_TYPE_ENUM, or MYSQL_TYPE_SET, 2nd byte contains length
    //    MYSQL_TYPE_NEWDECIMAL	                    2	        Precision, Scale
    //    MYSQL_TYPE_TIME2	                        1	        Length of microseconds
    //    MYSQL_TYPE_TIMESTAMP2	                    1	        Length of microseconds
    //    MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VAR_STRING	2	        Defined varchar length. If the value is > 255, length is stored in 2 bytes, otherwise 1 byte
//    pair<string , vector<uint8_t> > outValue;
    pair<int , vector<uint8_t> > outValueMetaData;
    vector<uint16_t> secValue;
    string typeName;
    if (fieldType == MYSQL_TYPE_NULL) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_NULL", secValue);

    } else if (fieldType == MYSQL_TYPE_TINY) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_TINY", secValue);

    } else if (fieldType == MYSQL_TYPE_SHORT) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_SHORT", secValue);

    } else if (fieldType == MYSQL_TYPE_YEAR) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_YEAR", secValue);

    } else if (fieldType == MYSQL_TYPE_INT24) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_INT24", secValue);

    } else if (fieldType == MYSQL_TYPE_LONG) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_LONG", secValue);

    } else if (fieldType == MYSQL_TYPE_LONGLONG) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_LONGLONG", secValue);
    } else if (fieldType == MYSQL_TYPE_FLOAT) {
        // 4 byte
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_FLOAT", secValue);
    } else if (fieldType == MYSQL_TYPE_DOUBLE) {
        // 8 byte
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_DOUBLE", secValue);
    } else if (fieldType == MYSQL_TYPE_DECIMAL) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 2, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_DECIMAL", secValue);
    } else if (fieldType == MYSQL_TYPE_NEWDECIMAL) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 2, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_NEWDECIMAL", secValue);
    } else if (fieldType == MYSQL_TYPE_VARCHAR) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 2, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_VARCHAR", secValue);
    } else if (fieldType == MYSQL_TYPE_BIT) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 2, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_BIT", secValue);
    } else if (fieldType == MYSQL_TYPE_ENUM) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 2, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_ENUM", secValue);
    } else if (fieldType == MYSQL_TYPE_SET) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 2, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_SET", secValue);

    } else if (fieldType == MYSQL_TYPE_TINY_BLOB) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_TINY_BLOB", secValue);
    } else if (fieldType == MYSQL_TYPE_MEDIUM_BLOB) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_MEDIUM_BLOB", secValue);
    } else if (fieldType == MYSQL_TYPE_LONG_BLOB) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_LONG_BLOB", secValue);
    } else if (fieldType == MYSQL_TYPE_BLOB) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_BLOB", secValue);
    } else if (fieldType == MYSQL_TYPE_VAR_STRING) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 2, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_VAR_STRING", secValue);
    } else if (fieldType == MYSQL_TYPE_STRING) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 2, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_STRING", secValue);
    } else if (fieldType == MYSQL_TYPE_GEOMETRY) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_GEOMETRY", secValue);
    } else if (fieldType == MYSQL_TYPE_DATE) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_DATE", secValue);
    } else if (fieldType == MYSQL_TYPE_NEWDATE) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_NEWDATE", secValue);
    } else if (fieldType == MYSQL_TYPE_TIMESTAMP) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_TIMESTAMP", secValue);

    }  else if (fieldType == MYSQL_TYPE_TIMESTAMP2) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_TIMESTAMP2", secValue);
    } else if (fieldType == MYSQL_TYPE_DATETIME) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_DATETIME", secValue);

    } else if (fieldType == MYSQL_TYPE_DATETIME2) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_DATETIME2", secValue);
    } else if (fieldType == MYSQL_TYPE_TIME) {
        secValue.push_back(65000);
        return make_pair("MYSQL_TYPE_TIME", secValue);

    } else if (fieldType == MYSQL_TYPE_TIME2) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
            printf(" second: %#x ", outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_TIME2", secValue);
    } else if (fieldType == MYSQL_TYPE_JSON) {
        outValueMetaData = takerBufferEventTypeAndMetaData(metaData, fieldType, seq, 1, metaDLen, pos);
        for (int i = 0; i < outValueMetaData.second.size(); ++i) {
            secValue.push_back(outValueMetaData.second[i]);
        }
        return make_pair("MYSQL_TYPE_JSON", secValue);
    } else {
        printf(" \n unknow fieldType: %d ", fieldType);
        printf("unknow mysql type!exit...");
        exit(1);
    }
};













