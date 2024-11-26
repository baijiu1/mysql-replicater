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
#include "binlogEvent.h"
#include "parser.h"


using namespace std;

typedef pair<int, vector<std::uint8_t> > (*MyFunctionType)(uint8_t*, struct ComBinlogDumpFields*, int*, int, int);



int InitMySQLTypeFuncMaps();
pair<int, vector<uint8_t> > tabkeBufferTypesHeader(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN);

pair<int, vector<uint8_t> > MysqlTypeNUMMFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeTinyFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeShortFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeYearFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeInt24Func(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeLongFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeLonglongFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeLongBlobFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeFloatFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeDoubleFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeDecimalFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeNewDecimalFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeVarcharFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeBitFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeEnumFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeSetFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeTinyBlobFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeMediumBlobFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeBlobFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeVarStringFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeStringFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeGeometryFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeDateFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeNewDateFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeTimestampFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeTimestamp2Func(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeDatetimeFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeDatetime2Func(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeTimeFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeTime2Func(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fieldN);
pair<int, vector<uint8_t> > MysqlTypeJSONFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN);
pair<int, vector<uint8_t> > MysqlTypedArrayFunc(uint8_t *received_data, struct ComBinlogDumpFields *binlogFields, int *recvLen, int eventLen, int fileN);
int binaryStringToDecimal(const std::string& binaryString);
//



#endif //MYSQL_REPLICATER_BINLOGTYPES_H
