//
// Created by 白杰 on 2024/5/18.
//

#ifndef MYSQL_REPLICATER_BINLOGTYPES_H
#define MYSQL_REPLICATER_BINLOGTYPES_H

#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <utility>
#include <cstdint>
#include "binlogEvent_test.h"
#include "parser.h"


using namespace std;

typedef pair<int, vector<std::uint8_t> > (*MyFunctionType)(vector<uint8_t>, struct ComBinlogDumpFields*, int*, int, int, int);



int InitMySQLTypeFuncMaps();
pair<int, vector<uint8_t> > tabkeBufferTypesHeader(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);

pair<int, vector<uint8_t> > MysqlTypeNUMMFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeTinyFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeShortFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeYearFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeInt24Func(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeLongFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeLonglongFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeLongBlobFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeFloatFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeDoubleFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeDecimalFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeNewDecimalFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeVarcharFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeBitFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeEnumFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeSetFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeTinyBlobFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeMediumBlobFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeBlobFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeVarStringFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeStringFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeGeometryFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeDateFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeNewDateFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeTimestampFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeTimestamp2Func(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeDatetimeFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeDatetime2Func(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeTimeFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeTime2Func(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypeJSONFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
pair<int, vector<uint8_t> > MysqlTypedArrayFunc(vector<uint8_t> received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN, int pos);
int binaryStringToDecimal(const std::string& binaryString);
//



#endif //MYSQL_REPLICATER_BINLOGTYPES_H
