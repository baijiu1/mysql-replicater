//
// Created by 白杰 on 2024/5/18.
//

#include "binlogTypes.h"
#include <string>

#include <map>
#include <iostream>
#include "binlogEvent.h"
#include <cmath>

using namespace std;

map<int, MyFunctionType> funcMap;
pair<int, vector<uint8_t> > outValue;
class Event *event;
extern map<int, vector<struct table_map_event_field_info> > field_info;

//    map     struct table_map_event_field_info;
//                    -------------
//                    |  table_id |
//                    -------------
//    -----------     -------------
//    table_id | --> |col_seq_num|
//    -----------     -------------
//                    -------------
//                    | col_type  |
//                    -------------
//                    -------------
//                    | meta_data |
//                    -------------
//                    -------------
//                    | event_type|
//                    -------------



// enum time2 都是有两次-----------field types: 0xfe ----------- 多执行了一次，后面看看啥问题

int InitMySQLTypeFuncMaps() {
    funcMap[MYSQL_TYPE_NULL] = &MysqlTypeNUMMFunc; // NULL-type字段
    funcMap[MYSQL_TYPE_TINY] = &MysqlTypeTinyFunc;   // ok TINYINT
    funcMap[MYSQL_TYPE_SHORT] = &MysqlTypeShortFunc;  // ok SMALLINT
    funcMap[MYSQL_TYPE_YEAR] = &MysqlTypeYearFunc; // ok  YEAR
    funcMap[MYSQL_TYPE_INT24] = &MysqlTypeInt24Func; // ok MEDIUMINT 3
    funcMap[MYSQL_TYPE_LONG] = &MysqlTypeLongFunc; // ok INTEGER
    funcMap[MYSQL_TYPE_LONGLONG] = &MysqlTypeLonglongFunc; // ok BIGINT
    funcMap[MYSQL_TYPE_FLOAT] = &MysqlTypeFloatFunc; // FLOAT    ok
    funcMap[MYSQL_TYPE_DOUBLE] = &MysqlTypeDoubleFunc; // DOUBLE或REAL字段   总是得到错误的数字
    funcMap[MYSQL_TYPE_DECIMAL] = &MysqlTypeDecimalFunc; // DECIMAL NUMERIC
    funcMap[MYSQL_TYPE_NEWDECIMAL] = &MysqlTypeNewDecimalFunc; // 大体 ok，待真正大数量下的检验
    funcMap[MYSQL_TYPE_VARCHAR] = &MysqlTypeVarcharFunc; // ok
    funcMap[MYSQL_TYPE_BIT] = &MysqlTypeBitFunc; // BIT ok
    funcMap[MYSQL_TYPE_ENUM] = &MysqlTypeEnumFunc; // ENUM ok大概，后面看看为什么多跑了一次
    funcMap[MYSQL_TYPE_SET] = &MysqlTypeSetFunc; // SET   没搞懂内容怎么计算的
    funcMap[MYSQL_TYPE_TINY_BLOB] = &MysqlTypeTinyBlobFunc;  // ok
    funcMap[MYSQL_TYPE_MEDIUM_BLOB] = &MysqlTypeMediumBlobFunc; // ok
    funcMap[MYSQL_TYPE_LONG_BLOB] = &MysqlTypeLongBlobFunc;  // ok
    funcMap[MYSQL_TYPE_BLOB] = &MysqlTypeBlobFunc; // ok
    funcMap[MYSQL_TYPE_VAR_STRING] = &MysqlTypeVarStringFunc; // VARCHAR
    funcMap[MYSQL_TYPE_STRING] = &MysqlTypeStringFunc; // CHAR ok
    funcMap[MYSQL_TYPE_GEOMETRY] = &MysqlTypeGeometryFunc; // Spatial  ok 待验证
    funcMap[MYSQL_TYPE_DATE] = &MysqlTypeDateFunc;  // ok  DATE
    funcMap[MYSQL_TYPE_NEWDATE] = &MysqlTypeNewDateFunc;
    funcMap[MYSQL_TYPE_TIMESTAMP] = &MysqlTypeTimestampFunc;// TIMESTAMP
    funcMap[MYSQL_TYPE_TIMESTAMP2] = &MysqlTypeTimestamp2Func;// TIMESTAMP    ok
    funcMap[MYSQL_TYPE_DATETIME] = &MysqlTypeDatetimeFunc; // DATETIME
    funcMap[MYSQL_TYPE_DATETIME2] = &MysqlTypeDatetime2Func; // DATETIME   ok
    funcMap[MYSQL_TYPE_TIME] = &MysqlTypeTimeFunc; // TIME
    funcMap[MYSQL_TYPE_TIME2] = &MysqlTypeTime2Func; // TIME   ok,精度后面多了个0，待解决
    funcMap[MYSQL_TYPE_JSON] = &MysqlTypeJSONFunc; // json ok 待验证
    funcMap[MYSQL_TYPE_TYPED_ARRAY] = &MysqlTypedArrayFunc; //
    return 0;
}

// 函数：将二进制字符串转换为十进制整数
int binaryStringToDecimal(const std::string& binaryString) {
    int decimalValue = 0;

    for (char bit : binaryString) {
        decimalValue = decimalValue * 2 + (bit - '0');
    }

    return decimalValue;
}


pair<int, vector<uint8_t> > MysqlTypeNUMMFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen,
                                              int eventLen, int fileN) {
    vector<uint8_t> fieldData;
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  1, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
    }

//    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.insert(make_pair(fileN, fieldData));
    return outValue;
//    return ;
}

pair<int, vector<uint8_t> > MysqlTypeTinyFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    if (binlogFields) {
//        binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.clear();
        outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  1, fileN);
//        binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.insert(make_pair(fileN, outValue.second[0]));
    } else {
        std::cerr << "Error: Attempt to use a null pointer." << std::endl;
    }
    return outValue;
//    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext[fileN] = outValue.second[0];


}

pair<int, vector<uint8_t> > MysqlTypeShortFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen,
                                               int eventLen, int fileN) {
    vector<uint8_t> fieldData;
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  2, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
    }
//    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.insert(make_pair(fileN, fieldData));
    return outValue;
}

pair<int, vector<uint8_t> > MysqlTypeYearFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen,
                                              int eventLen, int fileN) {
// Year类型，1个字节，记录年份，用一个字节记录，从1900年开始
//
//例如，当值为112时，表示112+1900 = 2012年
    printf("\n type year!!");
    vector<uint8_t> fieldData;
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  1, fileN);
//    outValue.second[0] = outValue.second[0] + 1900;
    printf("\n %d ", outValue.second[0] + 1900);

//    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.insert(make_pair(fileN, outValue.second[0]));
    return outValue;
}

pair<int, vector<uint8_t> > MysqlTypeLongFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen,
                                              int eventLen, int fileN) {
// 4
//                fieldData.clear();
    vector<uint8_t> fieldData;
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  4, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);

    }
//    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.insert(make_pair(fileN, fieldData));
    return outValue;
}

pair<int, vector<uint8_t> > MysqlTypeLongBlobFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen,
                                                  int eventLen, int fileN) {
// 4 byte

    if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0] != 0x04) {
        printf("not long blob type.exit...");
        exit(1);
    }
    vector<uint8_t> fieldData;
    fieldData.clear();
    int blobLength = 0;
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  4, fileN);
    swap(outValue.second[0], outValue.second[3]);
    swap(outValue.second[1], outValue.second[2]);
    string binaryString;
    for (const auto& hexValue : outValue.second) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binaryString += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binaryString += binary.to_string();
        }
    }
    bitset<32> binStr(binaryString);
    int strLength = binStr.to_ulong();

    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  strLength, fileN);

    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
        printf(" %c ", fieldData[i]);
    }
//    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.insert(make_pair(fileN, fieldData));
    return outValue;
//    return pair<int, vector<uint8_t>>();
}

pair<int, vector<uint8_t> > MysqlTypeInt24Func(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen,
                                               int eventLen, int fileN) {
    vector<uint8_t> fieldData;
    fieldData.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  3, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
    }
//    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.insert(make_pair(fileN, fieldData));
    return outValue;
}

pair<int, vector<uint8_t> > MysqlTypeLonglongFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
// 8
    vector<uint8_t> fieldData;
    fieldData.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  8, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
    }
//    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.insert(make_pair(fileN, fieldData));
    return outValue;
}
pair<int, vector<uint8_t> > MysqlTypeFloatFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    printf("\n MysqlTypeFloatFunc \n");
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  4, fileN);
    // 转换小端
    swap(outValue.second[0], outValue.second[3]);
    swap(outValue.second[1], outValue.second[2]);
    uint32_t hexVar = (outValue.second[0] << 24) | (outValue.second[1] << 16) | (outValue.second[2] << 8) | outValue.second[3];
    float V;
    memcpy(&V, (&hexVar), sizeof(float));

    return outValue;
}
pair<int, vector<uint8_t> > MysqlTypeDoubleFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
// 8 + ?
    printf("\n MysqlTypeDoubleFunc \n");
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  8, fileN);
    uint64_t hexVar;
    for (int i = 0; i < outValue.second.size(); ++i) {
        hexVar |= static_cast<uint64_t>(outValue.second[i]) << (i * 8);
    }
    printf(" size: %llu ", hexVar);

    double V;
    memcpy(&V, &hexVar, sizeof(double));
    printf("\n hexVar: %f \n", V);
    return outValue;
}

// MysqlTypeDecimalFunc/MysqlTypeNewDecimalFunc: Decimal类型，精度限制为65， 字节数与该类型的定义相关，可以参考用户手册 ，对该类型的解析主要是计算出其占用的字节数，调用libmysql.so库中的bin2decimal函数来实现解析。
pair<int, vector<uint8_t> > MysqlTypeDecimalFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {

    return outValue;
}
pair<int, vector<uint8_t> > MysqlTypeNewDecimalFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    // decimal(20,6)，20-6=14， decimal(M, D)
    // 其中小数部分为6 就对应上表中的3个字节
    // 整数部分为14，14-9=5，就是4个字节再加上表中的3个字节 3+4+3=10个字节
    //    Leftover Digits	Number of Bytes
    //    0	                    0
    //    1–2	                1
    //    3–4	                2
    //    5–6	                3
    //    7–9	                4
    //    DECIMAL最大占用多少字节应该是DECIMAL(65,0)
    //    65/9*4=>7*4+1=29字节.
    // decimal(10,2)    10/9*4 => 4 + 1 = 5;
    printf("\n--------------------------------decimal--------------------------------\n");
    string result;
    int M = field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0];
    int D = field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[1];
    printf("\n M定义长度： %d \n", M);
    printf("\n D定义长度： %d \n", D);
    int MLength = M - D;
    // （(15/9)4+3 + (10/9)4+1）=12
    // 先得到有几个9，然后MLength - 9 = 剩余的字节数，对应表中有多少byte
    int Mnum = MLength - ((MLength / 9) * 9);
    int Dnum = D - ((D / 9) * 9);
    int Mbyte, Dbyte;
    switch (Mnum) {
        case 0:
            Mbyte = 0;
            break;
        case 1:
        case 2:
            Mbyte = 1;
            break;
        case 3:
        case 4:
            Mbyte = 2;
            break;
        case 5:
        case 6:
            Mbyte = 3;
            break;
        case 7:
        case 8:
        case 9:
            Mbyte = 4;
            break;
    }
    switch (Dnum) {
        case 0:
            Dbyte = 0;
            break;
        case 1:
        case 2:
            Dbyte = 1;
            break;
        case 3:
        case 4:
            Dbyte = 2;
            break;
        case 5:
        case 6:
            Dbyte = 3;
            break;
        case 7:
        case 8:
        case 9:
            Dbyte = 4;
            break;
    }
//    int length = ((MLength / 9) * 4 + byte) + ((D / 9) * 4 + byte);
    int MLen = (MLength / 9) * 4 + Mbyte; // 多少个字节存储
    int DLen = (D / 9) * 4 + Dbyte;
    vector<uint8_t> fieldData;
    fieldData.clear();
    // 整数部分
    printf("\n M字节数： %d \n", MLen);
    printf("\n D字节数： %d \n", DLen);
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  MLen, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
//        printf(" %#x ", outValue.second[i]);
    }
    int mask;
    if ((outValue.second[0] & 0x80) == 0) {
        mask = -1;
    } else {
        mask = 0;
    }
    outValue.second[0] = outValue.second[0] ^ 0x80;
    // 字节数表示 -1表示第一位是mask，标识正负的
    // Mbyte-1 (MLength / 9) . (D / 9) Dbyte
    // 判断有多少个4字节存储，剩下的自行计算。每9个数字算4个字节
    // MResidueNum 为什么会算出随机值？？
//   int MResidueNum = (MLen - ((MLen / 4) * 4));
    uint32_t firstM;
    for (int i = 0; i < (MLen - ((MLen / 4) * 4)); ++i) {
        firstM |= static_cast<uint32_t>(outValue.second[i]) << (i * 8);
    }
    bitset<32> firM(firstM);
    int Mfirst = (int)firM.to_ullong() ^ mask;

    // 剩余的每4个字节为一个整体，与mask进行异或运算
    vector<uint64_t> Msecond;
    Msecond.clear();
    Msecond.reserve(MLen);
//    int Msecond;
    uint32_t uint32Values[20] = {0};
    for (int i = 0; i < (MLen / 4); ++i) {
        uint32Values[i] = (static_cast<uint32_t>(outValue.second[i * 4 + 0 + (MLen - ((MLen / 4) * 4))]) << 24) |
                          (static_cast<uint32_t>(outValue.second[i * 4 + 1 + (MLen - ((MLen / 4) * 4))]) << 16) |
                          (static_cast<uint32_t>(outValue.second[i * 4 + 2 + (MLen - ((MLen / 4) * 4))]) << 8) |
                          (static_cast<uint32_t>(outValue.second[i * 4 + 3 + (MLen - ((MLen / 4) * 4))]));
        Msecond[i] = uint32Values[i] ^ mask;
    }
    char formattedResultM[11]; // 10位数字 + 终止符
    for (int i = 0; i < (MLen / 4); ++i) {
        sprintf(formattedResultM, "%09llu", Msecond[i]);
//        printf(" %s ", formattedResultM);
        result += formattedResultM;
    }
//    printf("\n");
    result += ".";

    // 小数部分
    // 从左向右 先为4字节排列的9位数 最后才是零散的字节数
    outValue.second.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  DLen, fileN);

    // 剩余的每4个字节为一个整体，与mask进行异或运算
    vector<uint64_t> Dsecond;
    Dsecond.clear();
    Dsecond.reserve((DLen / 4));
//    int Msecond;
    uint64_t secondD;
    uint32_t uint32DValues[54] = {0};
    for (int i = 0; i < (DLen / 4); ++i) {
//        Dsecond.clear();
        uint32DValues[i] = (static_cast<uint32_t>(outValue.second[i * 4 + 0]) << 24) |
                          (static_cast<uint32_t>(outValue.second[i * 4 + 1]) << 16) |
                          (static_cast<uint32_t>(outValue.second[i * 4 + 2]) << 8) |
                          (static_cast<uint32_t>(outValue.second[i * 4 + 3]));
        Dsecond[i] = uint32DValues[i] ^ mask;
//        printf(" %llu ", Dsecond[i]);
    }
    char formattedResultD[11]; // 10位数字 + 终止符
    for (int i = 0; i < (DLen / 4); ++i) {
        sprintf(formattedResultD, "%09llu", Dsecond[i]);
//        printf(" %s ", formattedResultD);
        result += formattedResultD;
    }



    uint64_t firstD = 0;
    vector<uint64_t> Dfirst;
    Dfirst.reserve((DLen - ((DLen / 4) * 4)));

    if ((DLen - ((DLen / 4) * 4)) == 1) {
        firstD |= static_cast<uint64_t>(outValue.second[((DLen / 4) * 4)]);
    } else if ((DLen - ((DLen / 4) * 4)) == 2) {
        firstD = (static_cast<uint64_t>(outValue.second[((DLen / 4) * 4)]) << 8) |
                 (static_cast<uint64_t>(outValue.second[((DLen / 4) * 4) + 1]));
    } else if ((DLen - ((DLen / 4) * 4)) == 3) {
        firstD = (static_cast<uint64_t>(outValue.second[((DLen / 4) * 4) + 0]) << 16) |
                 (static_cast<uint64_t>(outValue.second[((DLen / 4) * 4) + 1]) << 8) |
                 (static_cast<uint64_t>(outValue.second[((DLen / 4) * 4) + 2]));
    } else if ((DLen - ((DLen / 4) * 4)) == 4) {
        firstD = (static_cast<uint64_t>(outValue.second[((DLen / 4) * 4) + 0]) << 24) |
                 (static_cast<uint64_t>(outValue.second[((DLen / 4) * 4) + 1]) << 16) |
                 (static_cast<uint64_t>(outValue.second[((DLen / 4) * 4) + 2]) << 8) |
                 (static_cast<uint64_t>(outValue.second[((DLen / 4) * 4) + 3]));
    }

    uint32_t DF = firstD ^ mask;
//    uint32_t DF = firstD ^ mask;
    char formattedResult[10];
    sprintf(formattedResult, "%09u", DF);
//    printf(" %s ", formattedResult);
    result += formattedResult;



//
//
    printf(" %s ", result.c_str());
    printf("\n--------------------------------decimal--------------------------------\n");
//    for (int i = 0; i < Msecond.size(); ++i) {
//        printf(" %llu ", Msecond[i]);
//    }
//    for (int i = 0; i < Dsecond.size(); ++i) {
//        printf(" %llu ", Dsecond[i]);
//    }
//    printf(" %d ", Dfirst);
    return outValue;
}

// MysqlTypeVarcharFunc/MysqlTypeBitFunc: byt_len = length%8==0? length/8 : (length/8 +  1, fileN);
// 例如，当定义为bit(M)时，length = M;将byt_len个字节中存储的数据转换为一个整数
pair<int, vector<uint8_t> > MysqlTypeVarcharFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    // 找到table_id，判断meta data中的长度，从field_info中获取
    // 转换为小端存储
    // 如果大于255，则使用两个字节存储长度
    // 读取前两个字节，swap后，转换为二进制，再转换为十进制即长度
    if (field_info.count(binlogFields->RowsEventHeader.tableIDNum) != 1) {
        printf(" table id not exists...");
        exit(1);
    }
    // 判断第几列 是否相等
    if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].file_seq_number == fileN) {
        if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data.size() > 1) {
            // 2个byte 转换小端序
            // 最大就是0xFFFF = 65535
            swap(field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0], field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[1]);
            // 转换为二进制
            string binStr;
            for (int i = 0; i < field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data.size(); ++i) {
                binStr += int2b(field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[i]);
            }
            // 这是元数据里的长度，不是实际长度，实际长度。实际长度也是根据这个来算出占两个字节
            int decimalValue = binaryStringToDecimal(binStr);

            vector<uint8_t> fieldData;
            fieldData.clear();
            outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  2, fileN);
            for (int i = 0; i < outValue.second.size(); ++i) {
                fieldData.push_back(outValue.second[i]);
            }
            swap(fieldData[0], fieldData[1]);
            string binaryString;
            for (const auto& hexValue : fieldData) {
                // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
                if (hexValue >= 0 && hexValue <= 9) {
                    std::bitset<4> binary(hexValue);
                    binaryString += binary.to_string();
                } else {
                    std::bitset<8> binary(hexValue);
                    binaryString += binary.to_string();
                }
            }
            bitset<32> binStr1(binaryString);
            vector<uint8_t> fieldDataContext;
            fieldDataContext.clear();
            outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  binStr1.to_ulong(), fileN);
            for (int i = 0; i < outValue.second.size(); ++i) {
                // 具体的值
                fieldDataContext.push_back(outValue.second[i]);
//                printf(" %c ", fieldDataContext[i]);
            }
//            binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.insert(make_pair(fileN, fieldDataContext));
        } else {

        }

    }




    return outValue;
}



pair<int, vector<uint8_t> > MysqlTypeBitFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    // （M+7）/8，M是字段定义的比特位数。
    // 找到元数据定义长度，然后计算存储长度


    // 取元数据的第二个字节，就是write rows里的长度
    int bitLen = field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[1];
    vector<uint8_t> fieldData;
    fieldData.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  bitLen, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
    }
//    binlogFields->RowsEventHeader.WriteRowsHeader.WriteRowsPayload.fieldContext.insert(make_pair(fileN, fieldData));


    // 求元数据长度
    swap(field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0], field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[1]);
    uint meta = 0;
    for (int i = 0; i < field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data.size(); ++i) {
        meta += field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[i];
    }
    // bit(M) nbits的值就是M
    uint nbits = ((meta >> 8) * 8) + (meta & 0xFF);

    return outValue;
}

//Enum类型，当该类型内的元素超过255个时，使用2个字节，否则使用1个字节表示，相应字节内转换为整数M，表示在enum中的第M个元素。
pair<int, vector<uint8_t> > MysqlTypeEnumFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    printf("\n ENUM!!! \n");
    vector<uint8_t> fieldData;
    fieldData.clear();
    int setLength = field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[1];
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  setLength, fileN);
    string binaryString;
    for (const auto& hexValue : outValue.second) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binaryString += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binaryString += binary.to_string();
        }
    }
    bitset<32> binStr(binaryString);
    int enumNum = binStr.to_ulong(); // 定义中的第几个
    printf(" \n enumNum: %d \n", enumNum);


    return outValue;
}

//SET类型，定义为SET(M)，M值为以下范围时：
//
//1 ~8，1个字节
//
//9~16，  2个字节
//
//17~24， 3个字节
//
//25~32， 4个字节
//
//33~64， 8个字节
//
//然后将相应字节内的数转换为整数即可
pair<int, vector<uint8_t> > MysqlTypeSetFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    // 元数据第二个表示长度
    printf("\n SET!!! \n");
    vector<uint8_t> fieldData;
    fieldData.clear();
    int setLength = field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[1];
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  setLength, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
//        blobLength += outValue.second[i];
        fieldData.push_back(outValue.second[i]);
        printf(" %#x ", fieldData[i]);
    }
//    string binaryString;
//    for (const auto& hexValue : outValue.second) {
//        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
//        if (hexValue >= 0 && hexValue <= 9) {
//            std::bitset<4> binary(hexValue);
//            binaryString += binary.to_string();
//        } else {
//            std::bitset<8> binary(hexValue);
//            binaryString += binary.to_string();
//        }
//    }
//    bitset<32> binStr(binaryString);
//    int strLength = binStr.to_ulong();







    return outValue;
}
pair<int, vector<uint8_t> > MysqlTypeTinyBlobFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0] != 0x01) {
        printf("not tiny blob type.exit...");
        exit(1);
    }
    vector<uint8_t> fieldData;
    fieldData.clear();
    int blobLength = 0;
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  1, fileN);
    string binaryString;
    for (const auto& hexValue : outValue.second) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binaryString += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binaryString += binary.to_string();
        }
    }
    bitset<32> binStr(binaryString);
    int strLength = binStr.to_ulong();

    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  strLength, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
//        blobLength += outValue.second[i];
        fieldData.push_back(outValue.second[i]);
//        printf(" %c ", fieldData[i]);
    }
    return outValue;
}
pair<int, vector<uint8_t> > MysqlTypeMediumBlobFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    // 3 个字节元数据
    // 3 个字节长度 小端存储
    // 判断元数据是不是等于3
    if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0] != 0x03) {
        printf("not medium blob type.exit...");
        exit(1);
    }
    vector<uint8_t> fieldData;
    fieldData.clear();
    int blobLength = 0;
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  3, fileN);
    swap(outValue.second[0], outValue.second[2]);
    string binaryString;
    for (const auto& hexValue : outValue.second) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binaryString += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binaryString += binary.to_string();
        }
    }
    bitset<32> binStr(binaryString);
    int strLength = binStr.to_ulong();
    printf("\n strLength: %d \n", strLength);
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  strLength, fileN);

//    printf(" \n%s\n ", binStr.c_str());
//    uint32_t result = (outValue.second[2] << 16) | (outValue.second[1] << 8) | outValue.second[0];
//    blobLength = result;

//    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  blobLength, fileN);
    printf("\n Medium blob!! \n");
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
        printf(" %c ", fieldData[i]);
    }
    return outValue;
}
pair<int, vector<uint8_t> > MysqlTypeBlobFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    // 所有blob类型都走这里，根据元数据来确定具体类型
    // 类型名                元数据     字符串长度（小端存储）
    // TINYBLOB/TINYTEXT      1           1
    // BLOB/TEXT              2           2
    // MEDIUMBLOB/MEDIUMTEXT  3           3
    // LONGBLOB/LONGTEXT      4           4
    printf("MysqlTypeBlobFunc\n");
//    for (int i = 0; i < *recvLen; ++i) {
//        printf(" %#x ", received_data[i]);
//    }

    if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0] == 0x01) {
        outValue = MysqlTypeTinyBlobFunc(received_data, binlogFields, recvLen, eventLen, fileN);
        return outValue;
    } else if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0] == 0x03) {
        outValue = MysqlTypeMediumBlobFunc(received_data, binlogFields, recvLen, eventLen, fileN);
        return outValue;
    } else if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0] == 0x04) {
        outValue = MysqlTypeLongBlobFunc(received_data, binlogFields, recvLen, eventLen, fileN);
        return outValue;
    }
//    printf(" %#x ", field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0]);
    if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0] != 0x02) {
        printf("not blob type.exit...");
//        exit(1);
    }
    vector<uint8_t> fieldData;
    fieldData.clear();
    int blobLength = 0;
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  2, fileN);
    swap(outValue.second[0], outValue.second[1]);
    for (int i = 0; i < outValue.second.size(); ++i) {
//        printf(" %#x ", outValue.second[i]);
    }
    string binaryString;
    for (const auto& hexValue : outValue.second) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binaryString += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binaryString += binary.to_string();
        }
    }
    bitset<32> binStr(binaryString);
    int strLength = binStr.to_ulong();
    printf("\n strLength: %d \n", strLength);
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  strLength, fileN);

//    printf(" \n%s\n ", binStr.c_str());
//    uint32_t result = (outValue.second[2] << 16) | (outValue.second[1] << 8) | outValue.second[0];
//    blobLength = result;

//    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  blobLength, fileN);
    printf("\n blob!! \n");
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
//        printf(" %c ", fieldData[i]);
    }
//    return make_pair(outValue.first, fieldData);
    return outValue;
}


pair<int, vector<uint8_t> > MysqlTypeVarStringFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
//包括char()、varchar()以及text类型，其处理方式相同，在record中首先根据其定义的长度，例如:
//
//对于varchar(10)，使用一个字节记录长度；而对于varchar(300)，则需要使用两个字节来记录字符串的长度；
//
//字符串“abcdef”，在record中被记录为“6abcdef”。
    vector<uint8_t> fieldData;
    outValue = event->LengthEnodedBinlogString(recvLen, received_data);
    fieldData.clear();
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
//        printf("\n char: %#x \n", fieldData[i]);
    }
    return outValue;
}
pair<int, vector<uint8_t> > MysqlTypeStringFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    // MYSQL_TYPE_STRING, MYSQL_TYPE_SET and MYSQL_TYPE_ENUM¶
    // 存储为固定长度的字符串，字符串的长度存储在表元数据的第二个字节中。
    // 这三种类型都作为MYSQL_TYPE_STRING存储在binlog中，字段的真实类型存储在元数据的第一个字节中。
    // 需要使用meta data辅助得知具体的长度
    // string<len>

    // 和set类型是共享同一个string类型，真实类型由这里判断
    if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0] == 0xf8) {
        outValue = MysqlTypeSetFunc(received_data, binlogFields, recvLen, eventLen, fileN);
        return outValue;
    }
    if (field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0] == 0xf7) {
        outValue = MysqlTypeEnumFunc(received_data, binlogFields, recvLen, eventLen, fileN);
        return outValue;
    }

    vector<uint8_t> fieldData;
    outValue = event->LengthEnodedBinlogString(recvLen, received_data);
    fieldData.clear();
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
//        printf(" %c ", fieldData[i]);
    }
    return outValue;
}
pair<int, vector<uint8_t> > MysqlTypeGeometryFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
    // len + row 先定义长度，后面跟着内容
    // 多个个字节定义多长度
    // 4个字节，从元数据中得到多长
    printf("\n MysqlTypeGeometryFunc \n");

    int geomLength = field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0];
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  geomLength, fileN);
    printf("\n geomLength: %d \n", geomLength);
    // 转换小端
    if (geomLength == 4) {
        swap(outValue.second[0], outValue.second[3]);
        swap(outValue.second[1], outValue.second[2]);
    }

    string binaryString;
    for (const auto& hexValue : outValue.second) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binaryString += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binaryString += binary.to_string();
        }
    }
    bitset<32> binStr(binaryString);
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  binStr.to_ulong(), fileN);

    vector<uint8_t> fieldData;
    fieldData.clear();
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
//        printf(" %c ", outValue.second[i]);
    }
    return outValue;
}
pair<int, vector<uint8_t> > MysqlTypeDateFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
//Date类型，3个字节，计算方法：
    vector<uint8_t> fieldData;
    vector<uint8_t> fieldDateData;
    fieldData.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  3, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
        printf("\n date: %#x \n", fieldData[i]);
    }
    // 转换小端序
    swap(fieldData[0], fieldData[2]);
//    std::reverse(fieldData.begin(), fieldData.end());
    string binaryString;
    for (const auto& hexValue : fieldData) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binaryString += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binaryString += binary.to_string();
        }
    }
    // 转换为二进制后。从右向左：1-5位表示日期，6-9位表示月份，剩余的表示年份
    // 从右往左取五位
    std::string dateDayTmp = binaryString.substr(binaryString.length() - 5, 5);
    bitset<5> dateDay(dateDayTmp);

    // 从右往左再取四位
    std::string dateMonthTmp = binaryString.substr(binaryString.length() - 9, 4);
    bitset<5> dateMonth(dateMonthTmp);
    // 取剩下的部分
    std::string dateYearTmp = binaryString.substr(0, binaryString.length() - 9);
    bitset<15> dateYear(dateYearTmp);

    printf("\n %lu - %lu - %lu \n", dateYear.to_ulong(), dateMonth.to_ulong(), dateDay.to_ulong());
    return outValue;


}
pair<int, vector<uint8_t> > MysqlTypeNewDateFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {

    return outValue;
}
pair<int, vector<uint8_t> > MysqlTypeTimestampFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
// timestamp时间戳类型，4个字节，直接进行类型的强制转换为整数
    vector<uint8_t> fieldData;
    fieldData.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  4, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
    }
    return outValue;
}

pair<int, vector<uint8_t> > MysqlTypeTimestamp2Func(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
// timestamp时间戳类型，4个字节，直接进行类型的强制转换为整数
// 不带精度的

    vector<uint8_t> fieldData;
    fieldData.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  4, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
//        printf(" %#x ", fieldData[i]);
    }
    string binaryString;
    for (const auto& hexValue : fieldData) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binaryString += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binaryString += binary.to_string();
        }
    }
    bitset<32> binStr(binaryString);


    int bitLen = field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0];
    if (bitLen == 0) {
        bitLen = 0;
        return outValue;
    } else if (bitLen == 1 || bitLen == 2) {
        bitLen = 1;
    } else if (bitLen == 3 || bitLen == 4) {
        bitLen = 2;
    } else if (bitLen == 5 || bitLen == 6) {
        bitLen = 3;
    }
    printf(" \n bitLne: %d \n", bitLen);
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  bitLen, fileN);
    fieldData.clear();
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
        printf(" %#x ", fieldData[i]);
    }
    string binStrPrecision;
    for (const auto& hexValue : fieldData) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binStrPrecision += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binStrPrecision += binary.to_string();
        }
    }
    int last = binaryStringToDecimal(binStrPrecision);
    auto tp = std::chrono::system_clock::from_time_t(binStr.to_ulong());
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm localTime = *std::localtime(&tt);
    printf(" %d - %d - %d %d:%d:%d.%d", (localTime.tm_year + 1900), (localTime.tm_mon + 1), localTime.tm_mday, localTime.tm_hour
            , localTime.tm_min, localTime.tm_sec, last);

//    printf(" %lu ", binStr.to_ulong());

//    std::cout << "Local time: ";
//    std::cout << (localTime.tm_year + 1900) << "-" << (localTime.tm_mon + 1) << "-" << localTime.tm_mday << " ";
//    std::cout << localTime.tm_hour << ":" << localTime.tm_min << ":" << localTime.tm_sec << std::endl;

    return outValue;
}

pair<int, vector<uint8_t> > MysqlTypeDatetimeFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
// Datetime类型，8个字节，直接类型转换为long long，
//
//例如对于 2011-08-27 19：32：46
//
//计算结果值为20110827193246
    vector<uint8_t> fieldData;
    fieldData.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  8, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
    }
    return outValue;
}

pair<int, vector<uint8_t> > MysqlTypeDatetime2Func(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
// 默认长度5字节，需要结合meta data精度来计算具体的长度，具体计算如下：
// 精度 字节数
//  0	0
//  1	1
//  2	1
//  3	2
//  4	2
//  5	3
//  6	3
// 先转换为二进制，从左向右第一位为最高位符号位（正数最高位为1，负数最高位为0）
// 17位：年月（年份 * 13+月份） 年=26233/13，月=26233%13 5位表示日期，5位表示时，6位表示分，6位表示秒
    printf("\n datetime2!! \n");
    // 不带精度的

    vector<uint8_t> fieldData;
    fieldData.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  5, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
//        printf(" %#x ", fieldData[i]);
    }
    string binaryString;
    for (const auto& hexValue : fieldData) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binaryString += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binaryString += binary.to_string();
        }
    }
    // 前18位表示年月（年份 * 13+月份），5位表示日期，5位表示时，6位表示分，6位表示秒

    // 6 位是秒
    std::string dateTimeSecondTmp = binaryString.substr(binaryString.length() - 6, 6);
    bitset<6> dateTimeSecond(dateTimeSecondTmp);
    // 6 位是分
    std::string dateTimeMinTmp = binaryString.substr(binaryString.length() - 12, 6);
    bitset<6> dateTimeMin(dateTimeMinTmp);
    // 5 位是时
    std::string dateTimeHourTmp = binaryString.substr(binaryString.length() - 17, 5);
    bitset<5> dateTimeHour(dateTimeHourTmp);

    // 5 位是日期
    std::string dateTimeDayTmp = binaryString.substr(binaryString.length() - 22, 5);
    bitset<5> dateTimeDay(dateTimeDayTmp);

    // 18 位是年月 减去最高位
    std::string dateTimeYearAndMonthTmp = binaryString.substr(1, 17);
    bitset<18> dateTimeYearAndMonth(dateTimeYearAndMonthTmp);

    // 年

    unsigned int dateTimeYear = (dateTimeYearAndMonth.to_ulong()) / 13;
    unsigned int dateTimeMonth = (dateTimeYearAndMonth.to_ulong()) % 13;



    // 带精度的
    // 精度部分是大端存储
    // 找到meta data中的精度部分，判断5 + M
    int bitLen = field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0];
    if (bitLen == 0) {
        bitLen = 0;
        return outValue;
    } else if (bitLen == 1 || bitLen == 2) {
        bitLen = 1;
    } else if (bitLen == 3 || bitLen == 4) {
        bitLen = 2;
    } else if (bitLen == 5 || bitLen == 6) {
        bitLen = 3;
    }
//    printf("\n bitLne: %#x ", bitLen);
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  bitLen, fileN);
    fieldData.clear();
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
//        printf(" %#x ", fieldData[i]);
    }
    // 转换小端存储
//    auto start = fieldData.begin();
//    auto end = fieldData.end() - 1;
//
//    // 当指针相遇或交错时停止循环
//    while (start < end) {
//        // 交换当前指针指向的元素
//        std::swap(*start, *end);
//        ++start;
//        --end;
//    }

    // 转换二进制 01 8c fc
    string binStr;
    for (const auto& hexValue : fieldData) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binStr += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binStr += binary.to_string();
        }
    }

    int last = binaryStringToDecimal(binStr);
    printf(" %d - %d - %lu %lu:%lu:%lu.%d", dateTimeYear, dateTimeMonth, dateTimeDay.to_ulong(), dateTimeHour.to_ulong()
            , dateTimeMin.to_ulong(), dateTimeSecond.to_ulong(), last);
    return outValue;
}

pair<int, vector<uint8_t> > MysqlTypeTimeFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
// Time类型，3个字节，计算方法：
//
//d_int = UCHAR(ptr) + (UCHAR(ptr+1)<<8) + (UCHAR(ptr+2)<<1 6, fileN);
//
//例如对于‘12:01:22’，计算结果为120122




    vector<uint8_t> fieldData;
    fieldData.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  3, fileN);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
    }
    return outValue;
}

pair<int, vector<uint8_t> > MysqlTypeTime2Func(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
// Time类型，3个字节，计算方法：
//
//d_int = UCHAR(ptr) + (UCHAR(ptr+1)<<8) + (UCHAR(ptr+2)<<1 6, fileN);
//
//例如对于‘12:01:22’，计算结果为120122
    vector<uint8_t> fieldData;
    fieldData.clear();
    outValue.second.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  3, fileN);
    long hexValue = (outValue.second[0] << 16) | (outValue.second[1] << 8) | outValue.second[2];
    long hex2 = 0x800000;

    // 计算差值的绝对值
    long difference = std::abs(hexValue - hex2);
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
//        printf(" %#x ", fieldData[i]);
    }
    bitset<32> binStr(difference);

//    printf(" binStr: %s ", binStr.to_string().c_str());
    // 6 位是秒
    std::string dateTimeSecondTmp = binStr.to_string().substr(binStr.to_string().length() - 6, 6);
    bitset<6> dateTimeSecond(dateTimeSecondTmp);
    // 6 位是分
    std::string dateTimeMinTmp = binStr.to_string().substr(binStr.to_string().length() - 12, 6);
    bitset<6> dateTimeMin(dateTimeMinTmp);
    // 12 位是分
    std::string dateTimeHourTmp = binStr.to_string().substr(0, binStr.to_string().length() - 12);
    bitset<24> dateTimeHour(dateTimeHourTmp);
    printf(" %lu:%lu:%lu ", dateTimeHour.to_ulong(), dateTimeMin.to_ulong(), dateTimeSecond.to_ulong());

    // 带精度的
    int bitLen = field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0];
    long orginHex = 0;
    if (bitLen == 0) {
        bitLen = 0;
//        return outValue;
    } else if (bitLen == 1 || bitLen == 2) {
        bitLen = 1;
        orginHex = 0x80000000;
//        hexValue <<= 4;
    } else if (bitLen == 3 || bitLen == 4) {
        bitLen = 2;
        orginHex = 0x8000000000;
//        hexValue <<= 8;
    } else if (bitLen == 5 || bitLen == 6) {
        bitLen = 3;
        orginHex = 0x800000000000;
//        hexValue <<= 12;
    }

    printf(" \n bitLne: %d \n", bitLen);
    outValue.second.clear();
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  bitLen, fileN);
    long precision;
    if (bitLen == 1) {
        precision = outValue.second[0];
    } else if (bitLen == 2) {
        // 4 位
        precision = (outValue.second[0] << 4) | outValue.second[1];
    } else if (bitLen == 3) {
        // 6位
        precision = (outValue.second[0] << 16) | (outValue.second[1] << 8) | outValue.second[2];
    }
    bitset<24> binStr1(precision);
    // 这里多了一个0，精度后面
    printf(" %lu:%lu:%lu.%lu ", dateTimeHour.to_ulong(), dateTimeMin.to_ulong(), dateTimeSecond.to_ulong(), binStr1.to_ulong());

    return outValue;
}

pair<int, vector<uint8_t> > tabkeBufferTypesHeader(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
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
            received_data[i -1] = received_data[ i + eventLen  - 1];
//            printf(" n %#x ", received_data[i - 1]);
        }
        *recvLen -= eventLen;
        return make_pair(eventLen, contextData);
    }
    return outValue;
};


pair<int, vector<uint8_t> > MysqlTypeJSONFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
//
    printf("\n MysqlTypeJSONFunc \n");
    int geomLength = field_info[binlogFields->RowsEventHeader.tableIDNum][fileN].field_meta_data[0];
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  geomLength, fileN);
    printf("\n geomLength: %d \n", geomLength);
    // 转换小端
    if (geomLength == 4) {
        swap(outValue.second[0], outValue.second[3]);
        swap(outValue.second[1], outValue.second[2]);
    }

    string binaryString;
    for (const auto& hexValue : outValue.second) {
        // 使用 std::bitset<8> 将每个 uint8_t 值转换为 8 位二进制字符串
        if (hexValue >= 0 && hexValue <= 9) {
            std::bitset<4> binary(hexValue);
            binaryString += binary.to_string();
        } else {
            std::bitset<8> binary(hexValue);
            binaryString += binary.to_string();
        }
    }
    bitset<32> binStr(binaryString);
    outValue = tabkeBufferTypesHeader(received_data, binlogFields, recvLen,  binStr.to_ulong(), fileN);

    vector<uint8_t> fieldData;
    fieldData.clear();
    for (int i = 0; i < outValue.second.size(); ++i) {
        fieldData.push_back(outValue.second[i]);
//        printf(" %hhu ", outValue.second[i]);
    }
    return outValue;
}


pair<int, vector<uint8_t> > MysqlTypedArrayFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN) {
//
//    vector<uint8_t> fieldData;
//    outValue = event->LengthEnodedBinlogString(recvLen, received_data);
//    fieldData.clear();
//    for (int i = 0; i < outValue.second.size(); ++i) {
//        fieldData.push_back(outValue.second[i]);
//    }
    return outValue;
}