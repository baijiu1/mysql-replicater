//
// Created by 白杰 on 2024/12/29.
//

#include "event.h"
#include "numeric.h"
#include "bpchar.h"
#include "date.h"


#include <string>
#include <stdio.h>

using namespace std;

int InitEventFuncMap() {
    typeEventFuncMap["numeric"] = &parserTypeForNumeric; // 剩下最后一个1B_E没有处理
    typeEventFuncMap["int"] = &parserTypeForInt;
    typeEventFuncMap["oid"] = &parserTypeForOid;
    typeEventFuncMap["bigint"] = &parserTypeForBigint;
    typeEventFuncMap["smallint"] = &parserTypeForSmallint;
    typeEventFuncMap["name"] = &parserTypeForName;
    typeEventFuncMap["float"] = &parserTypeForFloat;
    typeEventFuncMap["double"] = &parserTypeForDouble;
    typeEventFuncMap["varchar"] = &parserTypeForVarchar;
    typeEventFuncMap["text"] = &parserTypeForText;
    typeEventFuncMap["bytea"] = &parserTypeForBytea;
    typeEventFuncMap["char"] = &parserTypeForChar;
    typeEventFuncMap["bool"] = &parserTypeForBool;
    typeEventFuncMap["date"] = &parserTypeForDate;
    typeEventFuncMap["time"] = &parserTypeForTime;
    typeEventFuncMap["timestamp"] = &parserTypeForTimestamp;
    typeEventFuncMap["bpchar"] = &parserTypeForBpchar;
    typeEventFuncMap["timestampWithTimeZone"] = &parserTypeForTimestampWithTimeZone;
    typeEventFuncMap["interval"] = &parserTypeForInterval;
    typeEventFuncMap["uuid"] = &parserTypeForUuid;
    typeEventFuncMap["inet"] = &parserTypeForInet;
    typeEventFuncMap["cidr"] = &parserTypeForCidr;
    typeEventFuncMap["macaddr"] = &parserTypeForMacaddr;
    typeEventFuncMap["json"] = &parserTypeForJson;
    typeEventFuncMap["jsonb"] = &parserTypeForJsonb;
    typeEventFuncMap["hstore"] = &parserTypeForHstore;
    typeEventFuncMap["int4range"] = &parserTypeForInt4range;
    typeEventFuncMap["numrange"] = &parserTypeForNumrange;
    typeEventFuncMap["tsrange"] = &parserTypeForTsrange;
    typeEventFuncMap["tstzrange"] = &parserTypeForTstzrange;
    typeEventFuncMap["daterange"] = &parserTypeForDaterange;
    typeEventFuncMap["default"] = &parserTypeForDefault;
}



int parserTypeForNumeric(vector<uint8_t>& numricData, vector<uint8_t>& group, uint32_t *lpOff, string nextColumnAttalign) {
    // 物理格式
    // -------------------------------------------
    // | va_header   |    n_header    |    data  |
    // -------------------------------------------
    // | 1 byte     |    2 byte      |    ...    |
    // 得到完整的numric数据，包含了开头的长度，中间的标志位，后面的具体数据
    varlenaExtractNumeric(numricData, group, lpOff, nextColumnAttalign);
};

int parserTypeForSmallint(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 2) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
    uint16_t values = convertBigToLittleEndian16(fieldData, 0);
    printf("\nsmallINT: %u \n", values);
    *lpOff += dataLen;
};

int parserTypeForInt(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 4) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
    uint32_t values = convertBigToLittleEndian32(fieldData, 0);
    printf("\nINT: %u \n", values);
    *lpOff += dataLen;
};

int parserTypeForBigint(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 8) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
    uint64_t values = convertBigToLittleEndian64(fieldData, 0);
    printf("\nBigINT: %llu \n", values);
    *lpOff += dataLen;
};

int parserTypeForOid(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 4) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
    uint32_t values = convertBigToLittleEndian32(fieldData, 0);
    printf("\nOID: %u \n", values);
    *lpOff += dataLen;
};

int parserTypeForName(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 64) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节

    *lpOff += dataLen;
};

int parserTypeForFloat(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 4) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节

    *lpOff += dataLen;
};

int parserTypeForDouble(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    printf("\ndouble__begin");
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 8) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
//    for (int i = 0; i < fieldData.size(); ++i) {
//        printf(" %x ", fieldData[i]);
//    }
    uint64_t values = convertBigToLittleEndian64(fieldData, 0);

    double value;
    std::memcpy(&value, &values, sizeof(double));
    printf("\nDouble: %f \n", value);
    *lpOff += dataLen;
    printf("double__end\n");
};


int parserTypeForVarchar(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    varlenaExtractBpchar(fieldData, group, lpOff, nextColumnAttalign);
};

int parserTypeForText(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    varlenaExtractBpchar(fieldData, group, lpOff, nextColumnAttalign);
};

int parserTypeForBytea(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    varlenaExtractBpchar(fieldData, group, lpOff, nextColumnAttalign);
};

int parserTypeForChar(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    varlenaExtractNumeric(fieldData, group, lpOff, nextColumnAttalign);
};

int parserTypeForBool(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    fieldData.assign(group.begin(), group.begin() + 1);
    group.erase(group.begin(), group.begin() + 1); // 移除已取出的字节
    CopyAppend(*(bool *) (reinterpret_cast<char *>(fieldData.data())) ? "t" : "f");
    *lpOff += 1;
};

int parserTypeForDate(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 4) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
//    for (int i = 0; i < fieldData.size(); ++i) {
//        printf(" %x ", fieldData[i]);
//    }
    decode_date(reinterpret_cast<char *>(fieldData.data()), fieldData.size());
    *lpOff += dataLen;
};
int parserTypeForTime(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 8) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
    decode_time(reinterpret_cast<char *>(fieldData.data()), fieldData.size());
    *lpOff += dataLen;
    return 0;
};

int parserTypeForTimestamp(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 8) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
    decode_timestamp_internal(reinterpret_cast<char *>(fieldData.data()), fieldData.size(), false);
    *lpOff += dataLen;
};

int parserTypeForTimestampWithTimeZone(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    // 没有拿到时区信息
    uint32_t dataLen = TYPEALIGN(stoi(nextColumnAttalign), *lpOff + 8) - *lpOff;
    fieldData.assign(group.begin(), group.begin() + dataLen);
    group.erase(group.begin(), group.begin() + dataLen); // 移除已取出的字节
    decode_timestamp_internal(reinterpret_cast<char *>(fieldData.data()), fieldData.size(), true);
    *lpOff += dataLen;
};

int parserTypeForBpchar(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {
    varlenaExtractBpchar(fieldData, group, lpOff, nextColumnAttalign);
};

int parserTypeForInterval(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForInet(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForCidr(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForUuid(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForMacaddr(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForJson(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForJsonb(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForHstore(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForInt4range(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForNumrange(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForTsrange(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForTstzrange(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};
int parserTypeForDaterange(vector<uint8_t>& fieldData, vector<uint8_t >& group, uint32_t *lpOff, string nextColumnAttalign) {

};




int parserTypeForDefault(vector<uint8_t>& numricData, vector<uint8_t>& group, uint32_t *lpOff, string nextColumnAttalign) {
    printf("parserTypeForDefault \n");
}