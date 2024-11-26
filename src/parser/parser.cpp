//
// Created by 白杰 on 2024/5/7.
//

#include "parser.h"
#include <vector>
#include <map>
#include <utility>
#include "binlogEvent_test.h"
#include <bitset>

using namespace std;


vector<struct ComQueryFieldPayload*> PayloadFields;
vector<struct ComBinlogDumpFields> PayloadBinlogs;

map<int, string> PayloadRows;
struct ComQueryResultRows PayloadResultRows;

struct ComQueryField fieldMeta;
//class Event *event_parser;
class Event_test *event_parser;

int Parser::parserQuery(char *RecvBuffer, int recvSize) {
    // 先把头拿出来
    // 把内容 scan到一个数组里
    uint8_t data[recvSize - 5];
    int retHeader = handleBufferHeader(RecvBuffer, recvSize, data);
    if (retHeader != 0) {
        return 1;
    }
    // 所有都是fields
    int *sizeData = &recvSize - 5;
//    int retFields = handleBufferFields1(recvSize - 5, data, &fieldsHeader, &fieldsData1);
    int retFields = handleBufferFields(sizeData, data);
    if (retFields != 0) {
        return 1;
    }

    uint8_t *rowsData = data;
    readRows(sizeData, rowsData);
    return 0;
}

int* Parser::readRows(int * recvSize, uint8_t *rowsData) {
    int pos = 0;
    // 头 n: 跳过了多少个字节
    pair<int, vector<uint8_t> > outValue;
    outValue = SkipLengthEnodedStringRows(recvSize, rowsData, pos);
    if (outValue.first <= 0) {
        return 0;
    }
    if (outValue.first == 4 && pos == 0) {
        for (int i = 0; i < outValue.second.size(); ++i) {
            if (i == 3) {
                PayloadResultRows.rowsPacketNum = outValue.second[i];
                break;
            }
            PayloadResultRows.rowsPacketLen[i] = outValue.second[i];
        }
    }
    pos += outValue.first;
    for (int i = 0; i < fieldMeta.fieldsNum; ++i) {
        outValue = LengthEnodedRowsStringTest(recvSize, rowsData, pos, i);
        pos += outValue.first;
    }
    if (recvSize[0] > pos && rowsData[4] != 0xfe) {
        LengthEncodedRowsInt(rowsData);
        readRows(recvSize, rowsData);
    }
    return 0;
}

pair<int, vector<uint8_t> > Parser::SkipLengthEnodedStringRows(int *recvSize, uint8_t *data, int pos) {
    // get length data
    // num: 包的总长度
    // 这里主要是把包头去掉，直接取到0x03那里
    int num = LengthEncodedRowsInt(data);
    if (num < 1) {
        printf("num failed!");
        exit(1);
    }
//    printf(" num1: %d %d %#x ", num, pos, recvSize[0]);
    pair<int, vector<uint8_t> > outValue = takerFieldRowsData(num, recvSize, data, pos);
//    printf(" num2: %d %d %#x ", num, pos, recvSize[0]);
    return outValue;


}

struct ComQueryFieldPayload * Parser::readColumns(vector<uint8_t> *fieldsData) {



}

int Parser::handleBufferHeader(const char *RecvBuffer, int recvSize, uint8_t *data) {
    // 返回去除头的数组
    fieldMeta.packetLen = RecvBuffer[0];
    fieldMeta.packetNum = RecvBuffer[3];
    fieldMeta.fieldsNum = RecvBuffer[4];
    if (fieldMeta.fieldsNum < 0) {
        printf("scan field number failed! exit...\n");
        exit(1);
    }
    for (int i = 0; i < recvSize - 5; ++i) {
        data[i] = RecvBuffer[i + 5];
    }
    return 0;
}

int Parser::handleBufferFields(int * recvSize, uint8_t *data) {

    struct ComQueryFieldPayload *memFieldPayload = (struct ComQueryFieldPayload *) malloc(
            sizeof(struct ComQueryFieldPayload));
    int pos = 0;
    // 头 n: 跳过了多少个字节
    pair<int, vector<uint8_t> > outValue;
    outValue = SkipLengthEnodedString(memFieldPayload, recvSize, data, pos);
    if (outValue.first <= 0) {
        return 0;
    }
    if (outValue.first == 4 && pos == 0) {
        for (int i = 0; i < outValue.second.size(); ++i) {
            if (i == 3) {
                memFieldPayload->packetNum = outValue.second[i];
                break;
            }
            memFieldPayload->packetLen[i] = outValue.second[i];
        }
    }
    pos += outValue.first;
    // always "def"
    // int: 本个数据的长度 如本次循环的是catalog, 长度就为3
    // vector<uint8_t> 具体的值
    outValue = LengthEnodedString(memFieldPayload, recvSize, data, pos);
    memFieldPayload->catalogLen = outValue.first;
    for (int i = 0; i < outValue.second.size(); ++i) {
        memFieldPayload->catalog.push_back(outValue.second[i]);
    }
    pos += outValue.first;

    // schema
    outValue = LengthEnodedString(memFieldPayload, recvSize, data, pos);
    memFieldPayload->schemaLen = outValue.first;
    for (int i = 0; i < outValue.second.size(); ++i) {
        memFieldPayload->schema.push_back(outValue.second[i]);
    }

    pos += outValue.first;
    // table
    outValue = LengthEnodedString(memFieldPayload, recvSize, data, pos);
    memFieldPayload->tableNameLen = outValue.first;
    for (int i = 0; i < outValue.second.size(); ++i) {
        memFieldPayload->tableName.push_back(outValue.second[i]);
    }

    pos += outValue.first;
    // org table
    outValue = LengthEnodedString(memFieldPayload, recvSize, data, pos);
    memFieldPayload->physicalTableLen = outValue.first;
    for (int i = 0; i < outValue.second.size(); ++i) {
        memFieldPayload->physicalTable.push_back(outValue.second[i]);
    }

    pos += outValue.first;
    // column name
    outValue = LengthEnodedString(memFieldPayload, recvSize, data, pos);
    memFieldPayload->ColumnNameLen = outValue.first;
    for (int i = 0; i < outValue.second.size(); ++i) {
        memFieldPayload->ColumnName.push_back(outValue.second[i]);

    }

    pos += outValue.first;
    // phy column name
    outValue = LengthEnodedString(memFieldPayload, recvSize, data, pos);
    memFieldPayload->physicalColumnNameLen = outValue.first;
    for (int i = 0; i < outValue.second.size(); ++i) {
        memFieldPayload->physicalColumnName.push_back(outValue.second[i]);

    }

    pos += outValue.first;
    // 0x0c 12 length
    outValue = LengthEnodedString(memFieldPayload, recvSize, data, pos);
    memFieldPayload->fixedLen = outValue.first;
    if (memFieldPayload->fixedLen != 0x0c) {
        printf("not for select rows!");
        return 1;
    }
    pos += outValue.first;
    memFieldPayload->characterSet[0] = outValue.second[0];
    memFieldPayload->characterSet[1] = outValue.second[1];
    memFieldPayload->columnLength[0] = outValue.second[2];
    memFieldPayload->columnLength[1] = outValue.second[3];
    memFieldPayload->columnLength[2] = outValue.second[4];
    memFieldPayload->columnLength[3] = outValue.second[5];
    memFieldPayload->type[0] = outValue.second[6];
    memFieldPayload->flags[0] = outValue.second[7];
    memFieldPayload->flags[1] = outValue.second[8];
    memFieldPayload->decimals[0] = outValue.second[9];
    pos += 10;
    PayloadFields.push_back(memFieldPayload);
    free(memFieldPayload);
    // if more data, command was field list
    if (recvSize[0] > pos && memFieldPayload->packetNum <= fieldMeta.fieldsNum) {
        // 判断第几次第时候终止循环
        LengthEncodedInt(data);
        handleBufferFields(recvSize, data);
    }

    return 0;
};

pair<int, vector<uint8_t> > Parser::SkipLengthEnodedString(struct ComQueryFieldPayload *memFieldPayload, int *recvSize, uint8_t *data, int pos) {
    // get length data
    // num: 包的总长度
    // 这里主要是把包头去掉，直接取到0x03那里

    int num = LengthEncodedInt(data);
    if (num < 0) {
        printf("skip num failed!");
        exit(1);
    }
//    printf(" num1: %d %d %#x ", num, pos, recvSize[0]);
    pair<int, vector<uint8_t> > outValue = takerFieldData(memFieldPayload, num, recvSize, data, pos);
//    printf(" num2: %d %d %#x ", num, pos, recvSize[0]);
    return outValue;
}

pair<int, vector<uint8_t> > Parser::LengthEnodedString(struct ComQueryFieldPayload *memFieldPayload, int *recvSize, uint8_t *data, int pos) {
    // get length
    int num = LengthEncodedInt(data);
    printf(" num!: %#x ", num);
    if (num < 0) {
        printf("encode string num failed!");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue = takerFieldData(memFieldPayload, num, recvSize, data, pos);
    return outValue;
}

pair<int, vector<uint8_t> > Parser::LengthEnodedRowsString(int *recvSize, uint8_t *data, int pos) {
    // get length
//    printf(" data: %#x ", data[0]);
    int num = LengthEncodedRowsInt(data);
    printf(" num: %#x ", num);
    if (num < 1) {
        printf("encode rows string num failed!");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue = takerFieldRowsData(num, recvSize, data, pos);
    return outValue;
}

pair<int, vector<uint8_t> > Parser::takerFieldData(struct ComQueryFieldPayload *memFieldPayload, int num, int *recvSize, uint8_t *data, int pos) {
    int n = 0;
    if (pos == 0) {
        // 头，跳过
        vector<uint8_t> headerData; // 具体的内容
        headerData.reserve(4);
        for (int i = 0; i < 4; ++i) {
            headerData.push_back(data[i]);
        }
        for (int i = 0; i < recvSize[0]; ++i) {
            data[i] = data[i + 4];
        }
        recvSize[0] -= 4;
        // 如果是头的话，表示跳过了多少长度
        // 如果是内容，则表示长度
        n = 4;
        return make_pair(n, headerData);
    }
    // 内容
    // num: 内容开始长度
    vector<uint8_t> contextData; // 具体的内容
    if (pos > 0 && num >= 0) {
//        printf(" num: %d ", num);
        for (int i = 1; i <= num; ++i) {
            contextData.push_back(data[i]);
        }
        // 减去num，更新data数据
        // 这里更新data到以下一个长度开头
        for (int i = 1; i < recvSize[0]; ++i) {
            data[i - 1] = data[ i + num  ];
        }
        recvSize[0] -= num;
        return make_pair(num, contextData);
    }
};

pair<int, vector<uint8_t> > Parser::takerFieldRowsData(int num, int *recvSize, uint8_t *data, int pos) {
    int n = 0;
    if (pos == 0) {
        for (int i = 0; i < recvSize[0]; ++i) {
//            printf(" %#x ", data[i]);
        }
        // 头，跳过
        vector<uint8_t> headerData; // 具体的内容
        headerData.reserve(4);
        for (int i = 0; i < 4; ++i) {
            headerData.push_back(data[i]);
//            printf(" %#x ", data[i]);
        }
        for (int i = 0; i < recvSize[0]; ++i) {
            data[i] = data[i + 4];
        }
        recvSize[0] -= 4;
        // 如果是头的话，表示跳过了多少长度
        // 如果是内容，则表示长度
        n = 4;
        return make_pair(n, headerData);
    }
    // 内容
    // num: 内容开始长度
    vector<uint8_t> contextData; // 具体的内容
    if (pos > 0 && num >= 0) {
//        printf(" num: %d ", num);
        for (int i = 1; i <= num; ++i) {
            contextData.push_back(data[i]);
//            printf(" %#x ", data[i]);
        }
        // 减去num，更新data数据
        // 这里更新data到以下一个长度开头
        for (int i = 1; i < recvSize[0]; ++i) {
            // 这里
            data[i -1] = data[ i + num ];
//            printf(" %#x ", data[i]);
        }
        recvSize[0] -= num;
        return make_pair(num, contextData);
    }
};


int Parser::LengthEncodedInt(uint8_t *data) {
    int n = 0, num = 0;
    switch (data[0]) {
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
    num = data[0];
    return num;
};

int Parser::LengthEncodedRowsInt(uint8_t *data) {
    int n = 0, num = 0;
    switch (data[0]) {
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
    num = data[0];
    return num;
};


pair<int, vector<uint8_t> > Parser::LengthEnodedRowsStringTest(int *recvSize, uint8_t *data, int pos, int fieldI) {
    // get length
//    printf(" data: %#x ", data[0]);
    int num = LengthEncodedRowsInt(data);
//    printf(" num: %#x ", num);
    if (num < 0) {
        printf("encode rows string test num failed!");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue = takerFieldRowsDataTest(num, recvSize, data, pos, fieldI);

    return outValue;
}

pair<int, vector<uint8_t> > Parser::takerFieldRowsDataTest(int num, int *recvSize, uint8_t *data, int pos, int fieldI) {
    int n = 0;
    if (pos == 0) {
        for (int i = 0; i < recvSize[0]; ++i) {
//            printf(" %#x ", data[i]);
        }
        // 头，跳过
        vector<uint8_t> headerData; // 具体的内容
        headerData.reserve(4);
        for (int i = 0; i < 4; ++i) {
            headerData.push_back(data[i]);
//            printf(" %#x ", data[i]);
        }
        for (int i = 0; i < recvSize[0]; ++i) {
            data[i] = data[i + 4];
        }
        recvSize[0] -= 4;
        // 如果是头的话，表示跳过了多少长度
        // 如果是内容，则表示长度
        n = 4;
        return make_pair(num, headerData);
    }
    // 内容
    // num: 内容开始长度
    vector<uint8_t> contextData;
//    contextData->clear();
    string cData;

    if (pos > 0 && num >= 0) {
//        printf(" num: %d ", num);
        for (int i = 0; i < num; ++i) {
            contextData.push_back(data[i + 1]);
            cData += data[i + 1];
//            contextData[i] = data[i];
//            printf(" %#x ", cData[i]);
        }
        PayloadFields[fieldI]->ResultRows.rowsData = cData;

        // 减去num，更新data数据
        // 这里更新data到以下一个长度开头
        for (int i = 1; i < recvSize[0]; ++i) {
            // 这里
            data[i -1] = data[ i + num ];
//            printf(" %#x ", data[i - 1]);
        }
        recvSize[0] -= num;
        return make_pair(num, contextData);
    }
};
pair<int, vector<uint8_t> > Parser::LengthEnodedBinlogString(uint8_t *received_data, int pos, int *recvLen) {
    int num = LengthEnodedBinlogInt(received_data);
    printf(" num1: %#x ", num);
    if (num < 0) {
        printf("encode binlog string num failed!");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue = takerBinlogData(num, received_data, pos, recvLen);
    return outValue;

}

// 将十六进制字符串转换为二进制字符串
std::string hexToBinary(const std::string& hexString) {
    unsigned long decimalValue = std::stoul(hexString, nullptr, 16); // 将十六进制字符串转换为无符号长整数
    std::bitset<64> binary(decimalValue); // 使用 64 位的 bitset 来表示二进制数
    return binary.to_string();
}

int Parser::LengthEnodedBinlogInt(uint8_t *received_data) {
    // 函数返回第一个字节的长度
    int n = 0;
    uint32_t num = 0;
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
    // 3个byte 小端存储 需要reverse
    vector<uint8_t> pckLen;
    pckLen.reserve(3);
    for (int i = 0; i < 3; ++i) {
        pckLen.push_back(received_data[i]);

    }
    if (pckLen.size() > 1) {
        // 交换
        swap(pckLen[0], pckLen[2]);
    }
//    printf(" \n%s\n ", binStr.c_str());
    uint32_t result = (received_data[2] << 16) | (received_data[1] << 8) | received_data[0];
    num =  result;
//    printf("\n pckLenNum first: %d ", num);
    return num;

}

pair<int, vector<uint8_t> > Parser::takerBinlogData(int num, uint8_t* received_data, int pos, int *recvLen) {
    // struct ComQueryFieldPayload *memFieldPayload, int num, int *recvSize, uint8_t *data, int pos
    // 这里返回下一个binlog events的内容
    int n = 0;

    if (pos == 0) {
//        printf("binlog recv %d ", recvLen[0]);
        // 头，跳过
        vector<uint8_t> headerData; // 具体的内容
        headerData.reserve(5);
        for (int i = 0; i < 5; ++i) {
            headerData.push_back(received_data[i]);
//            printf(" %#x ", headerData[i]);
        }

        for (int i = 0; i < recvLen[0]; ++i) {
            received_data[i] = received_data[i + 5];
//            printf(" %#x ", received_data[i]);
        }
        recvLen[0] -= 5;
        // 如果是头的话，表示跳过了多少长度
        // 如果是内容，则表示长度
        n = 5;
        return make_pair(num, headerData);
    }

    vector<uint8_t> contextData;
    if (pos > 0 && num >= 0) {
//        printf(" num: %d ", num);
        for (int i = 0; i < num; ++i) {
            contextData.push_back(received_data[i]);
//            contextData[i] = data[i];
//            printf(" %#x ", contextData[i]);
        }

        // 减去num，更新data数据
        // 这里更新data到以下一个长度开头

        for (int i = 1; i < (*recvLen); ++i) {
            // 这里
            received_data[i -1] = received_data[ i + num ];
//            printf(" %#x ", data[i - 1]);
        }
        *recvLen -= num;
        return make_pair(num, contextData);
    }
};

// 只解析header
pair<int, vector<uint8_t> > Parser::LengthEnodedBinlogHeaderString(uint8_t *received_data, int pos, int *recvLen, int header) {
    int num = LengthEnodedBinlogInt(received_data);
    printf(" nummm: %#x ", num);
    if (num < 0) {
        printf("encode binlog header string num failed!");
        exit(1);
    }
    pair<int, vector<uint8_t> > outValue = takerBinlogDataHeader(num, received_data, pos, recvLen, header);
    return outValue;
}
pair<int, vector<uint8_t> > Parser::takerBinlogDataHeader(int num, uint8_t* received_data, int pos, int *recvLen, int header) {
    // struct ComQueryFieldPayload *memFieldPayload, int num, int *recvSize, uint8_t *data, int pos
    // 这里返回下一个binlog events的内容
    int n = 0;
    if (pos == 0) {
//        for (int i = 0; i < *recvLen; ++i) {
//            printf(" %#x ", received_data[i]);
//        }
        // 头，跳过
        vector<uint8_t> headerData; // 具体的内容
        headerData.reserve(5);
        for (int i = 0; i < 5; ++i) {
            headerData.push_back(received_data[i]);
//            printf(" %#x ", headerData[i]);
        }
        for (int i = 0; i < *recvLen; ++i) {
            received_data[i] = received_data[i + 5];
        }
        *recvLen -= 5;
        // 如果是头的话，表示跳过了多少长度
        // 如果是内容，则表示长度
        n = 5;
        return make_pair(n, headerData);
    }

    vector<uint8_t> contextData;
    if (pos > 0 && header >= 0) {
//        printf(" header: %d ", header);
        for (int i = 0; i < header; ++i) {
            contextData.push_back(received_data[i]);
//            contextData[i] = data[i];
//            printf(" %#x ", received_data[i]);
        }

        // 减去num，更新data数据
        // 这里更新data到以下一个长度开头
        for (int i = 0; i < *recvLen; ++i) {
            // 这里
            received_data[i -1] = received_data[ i + header - 1 ];
//            printf(" n %#x ", received_data[i - 1]);
        }
        *recvLen -= header;
        return make_pair(header, contextData);
    }
};

pair<int, vector<uint8_t> > Parser::skipLengthEnodedBinlogString(uint8_t *received_data, int pos, int *recvLen) {
    // 跳过长度头
    // get length data
    // num: 包的总长度
    // 这里主要是把包头去掉，直接取到0x03那里

    int num = LengthEnodedBinlogInt(received_data);
    if (num < 0) {
        printf("skip num failed!");
        exit(1);
    }

//    printf(" num1: %d %d %#x ", num, pos, recvLen[0]);
    pair<int, vector<uint8_t> > outValue = takerBinlogData(num, received_data, pos, recvLen);
//    printf(" num2: %d %d %#x ", num, pos, recvSize[0]);
    return outValue;
};

pair<int, vector<uint8_t> > tabkeBufferQueryEventHeader(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen) {
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
        for (int i = 0; i < *recvLen; ++i) {
            // 这里
            received_data[i -1] = received_data[ i + eventLen - 1 ];
//            printf(" n %#x ", received_data[i - 1]);
        }
        *recvLen -= eventLen;
        return make_pair(eventLen, contextData);
    }
};

int Parser::handleBufferBinlogEvents(uint8_t *received_data, int *recvLen) {
//    int<4>	timestamp	seconds since unix epoch
//    int<1>	event_type	See mysql::binlog::event::Log_event_type
//    int<4>	server-id	server-id of the originating mysql-server. Used to filter out events in circular replication
//    int<4>	event-size	size of the event (header, post-header, body)
//    if binlog-version > 1 {
//        int<4>	log-pos	position of the next event
//        int<2>	flags	See Binlog Event Header Flags

    struct ComBinlogDumpFields *binlogFields = (struct ComBinlogDumpFields *) malloc(sizeof(struct ComBinlogDumpFields) * 2);
    event_parser->Init(binlogFields);
    if (binlogFields == nullptr) {
        printf("\n Failed to allocate memory! \n");
        exit(EXIT_FAILURE);
    }
    // 跳过头
    int pos = 0;
    printf(" \n ");
    printf("\n received_data[4]: %#x \n", received_data[4]);
    if (received_data[4] != 0) {
        printf("binlog recv not OK!may be recv is all right.continue...");
//        goto __finish;
        free(binlogFields);
        return 0;
    }

    // 头 n: 跳过了多少个字节
    pair<int, vector<uint8_t> > outValue;
    outValue = skipLengthEnodedBinlogString(received_data, pos, recvLen);
    // 这里有问题，应该是三个字节加起来是总长度
    int pckHeaderLen = outValue.first;
    if (outValue.first <= 0) {
        return 0;
    }

    for (int i = 0; i < outValue.second.size(); ++i) {
        if (i == 3) {
            binlogFields->packetNum = outValue.second[i];
        } else if (i == 4) {
            binlogFields->packetStat = outValue.second[i];
            break;
        }
        binlogFields->packetLen[i] = outValue.second[i];
    }

    pos += outValue.first;

    // timestamp
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->timestamp[i] = outValue.second[i];
    }

    // event type
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 1);
    binlogFields->eventType = outValue.second[0];

    // server-id
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->serverID[i] = outValue.second[i];
    }

    // event-size
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->eventSize[i] = outValue.second[i];
    }

    // log-pos
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 4);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->logPos[i] = outValue.second[i];
    }

    // flags
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader(received_data, binlogFields, recvLen, 2);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->flags[i] = outValue.second[i];
    }
//    outValue = LengthEnodedBinlogHeaderString(received_data, pos, recvLen, 2);
//
//    for (int i = 0; i < outValue.second.size(); ++i) {
//        binlogFields->flags[i] = outValue.second[i];
////        printf(" %#x ", binlogFields->flags[i]);
//    }
    // initialize struct binlogFields for every element

    // outValue.first string<EOF>这种包用的到
//    binlogFields->eventType = 2;
//    printf("\n binlogFields->eventType: %#x ", binlogFields->eventType);
//    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.clear();



//    event_parser->parseEvent(binlogFields->eventType, received_data, binlogFields, recvLen, pckHeaderLen);




//    printf("\n PayloadBinlogs.push_back(*binlogFields);\n");
    // 剩余2个元素异常了 12次
    PayloadBinlogs.push_back(*binlogFields);
//    printf("\n end PayloadBinlogs.push_back(*binlogFields);\n");
    free(binlogFields);
//    binlogFields = nullptr;
    printf("\n");
    // more data
//    if (*recvLen > 0) {
//        handleBufferBinlogEvents(received_data, recvLen);
//    }
    if (received_data[4] == 0) {
        handleBufferBinlogEvents(received_data, recvLen);
    }
    return 0;

}

pair<int, vector<uint8_t> > Parser::takerBinlogData_test(int num, vector<uint8_t> received_data, int pos, int *recvLen) {
    // struct ComQueryFieldPayload *memFieldPayload, int num, int *recvSize, uint8_t *data, int pos
    // 这里返回下一个binlog events的内容
    int n = 0;

    if (pos == 0) {
//        printf("binlog recv %d ", recvLen[0]);
        // 头，跳过
        vector<uint8_t> headerData; // 具体的内容
        headerData.reserve(5);
        for (int i = 0; i < 5; ++i) {
            headerData.push_back(received_data[i]);
//            printf(" %#x ", headerData[i]);
        }

        for (int i = 0; i < recvLen[0]; ++i) {
            received_data[i] = received_data[i + 5];
//            printf(" %#x ", received_data[i]);
        }
        recvLen[0] -= 5;
        // 如果是头的话，表示跳过了多少长度
        // 如果是内容，则表示长度
        n = 5;
        return make_pair(num, headerData);
    }

    vector<uint8_t> contextData;
    if (pos > 0 && num >= 0) {
//        printf(" num: %d ", num);
        for (int i = 0; i < num; ++i) {
            contextData.push_back(received_data[i]);
//            contextData[i] = data[i];
//            printf(" %#x ", contextData[i]);
        }

//        // 减去num，更新data数据
//        // 这里更新data到以下一个长度开头
//
//        for (int i = 1; i < (*recvLen); ++i) {
//            // 这里
//            received_data[i -1] = received_data[ i + num ];
////            printf(" %#x ", data[i - 1]);
//        }
//        *recvLen -= num;
        return make_pair(num, contextData);
    }
};

pair<int, vector<uint8_t> > tabkeBufferQueryEventHeader_test(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int pos) {
    vector<uint8_t> contextData;
    if (eventLen >= 0) {
        for (int i = 0; i < eventLen; ++i) {
            contextData.push_back(received_data[i + pos]);
//            binlogFields->queryHeader.slaveProxyID[i] = received_data[i];
//            contextData[i] = data[i];
//            printf(" %#x ", binlogFields->position[i]);
        }
        // 减去num，更新data数据
        // 这里更新data到以下一个长度开头
//        for (int i = 0; i < *recvLen; ++i) {
//            // 这里
//            received_data[i -1] = received_data[ i + eventLen - 1 ];
////            printf(" n %#x ", received_data[i - 1]);
//        }
//        *recvLen -= eventLen;
        return make_pair(eventLen, contextData);
    }
};


int Parser::LengthEnodedBinlogInt_test(vector<uint8_t> received_data) {
    // 函数返回第一个字节的长度
    int n = 0;
    uint32_t num = 0;
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
    // 3个byte 小端存储
    swap(received_data[0], received_data[2]);

    uint32_t result = (received_data[0] << 16) | (received_data[1] << 8) | received_data[2];
    num =  result;
    return num;

}

pair<int, vector<uint8_t> > Parser::skipLengthEnodedBinlogString_test(vector<uint8_t> received_data, int pos, int *recvLen) {
    // 跳过长度头
    // get length data
    // num: 包的总长度
    // 这里主要是把包头去掉，直接取到0x03那里

    int num = LengthEnodedBinlogInt_test(received_data);
    if (num < 0) {
        printf("skip num failed!");
        exit(1);
    }

//    printf(" num1: %d %d %#x ", num, pos, recvLen[0]);
    pair<int, vector<uint8_t> > outValue = takerBinlogData_test(num, received_data, pos, recvLen);
//    printf(" num2: %d %d %#x ", num, pos, recvSize[0]);
    return outValue;
};

int Parser::handleBufferBinlogEvents_test(vector<uint8_t> received_data, int *recvLen) {
//    int<4>	timestamp	seconds since unix epoch
//    int<1>	event_type	See mysql::binlog::event::Log_event_type
//    int<4>	server-id	server-id of the originating mysql-server. Used to filter out events in circular replication
//    int<4>	event-size	size of the event (header, post-header, body)
//    if binlog-version > 1 {
//        int<4>	log-pos	position of the next event
//        int<2>	flags	See Binlog Event Header Flags

    struct ComBinlogDumpFields *binlogFields = (struct ComBinlogDumpFields *) malloc(sizeof(struct ComBinlogDumpFields) * 2);
    for (int i = 0; i < received_data.size(); ++i) {
        printf(" %#x ", received_data[i]);
    }
    event_parser->Init(binlogFields);
    if (binlogFields == nullptr) {
        printf("\n Failed to allocate memory! \n");
        exit(EXIT_FAILURE);
    }
    // 跳过头
    int pos = 0;
    printf("\n received_data[4]: %#x \n", received_data[4]);
    if (received_data[4] != 0) {
        printf("binlog recv not OK!may be recv is all right.continue...");
//        goto __finish;
        free(binlogFields);
        return 0;
    }

    // 头 n: 跳过了多少个字节
    pair<int, vector<uint8_t> > outValue;
    int allLen = LengthEnodedBinlogInt_test(received_data);
    printf("\n segment lengt: %d \n", allLen);
//    outValue = skipLengthEnodedBinlogString_test(received_data, pos, recvLen);
    outValue = tabkeBufferQueryEventHeader_test(received_data, binlogFields, recvLen, 5, pos);
    // 这里有问题，应该是三个字节加起来是总长度
    int pckHeaderLen = allLen;
    if (outValue.first <= 0) {
        return 0;
    }

    for (int i = 0; i < outValue.second.size(); ++i) {
        if (i == 3) {
            binlogFields->packetNum = outValue.second[i];
        } else if (i == 4) {
            binlogFields->packetStat = outValue.second[i];
            break;
        }
        binlogFields->packetLen[i] = outValue.second[i];
    }

    pos += outValue.first;

    // timestamp
    outValue = tabkeBufferQueryEventHeader_test(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->timestamp[i] = outValue.second[i];
    }

    // event type
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader_test(received_data, binlogFields, recvLen, 1, pos);
    binlogFields->eventType = outValue.second[0];

    // server-id
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader_test(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->serverID[i] = outValue.second[i];
    }

    // event-size
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader_test(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->eventSize[i] = outValue.second[i];
    }

    // log-pos
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader_test(received_data, binlogFields, recvLen, 4, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->logPos[i] = outValue.second[i];
    }

    // flags
    pos += outValue.first;
    outValue = tabkeBufferQueryEventHeader_test(received_data, binlogFields, recvLen, 2, pos);
    for (int i = 0; i < outValue.second.size(); ++i) {
        binlogFields->flags[i] = outValue.second[i];
    }
    pos += outValue.first;
    event_parser->parseEvent(binlogFields->eventType, received_data, binlogFields, recvLen, pckHeaderLen, pos);

    PayloadBinlogs.push_back(*binlogFields);
    free(binlogFields);

    return 0;

}