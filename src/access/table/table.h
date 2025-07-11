//
// Created by 白杰 on 2024/12/14.
//

#ifndef MYSQL_REPLICATER_TABLE_H
#define MYSQL_REPLICATER_TABLE_H
#define PG

#include <string>
#include <vector>
#include <map>
#include "page.h"
#include "system.h"
#include "type.h"

using namespace std;


#ifdef PG
#define HeapPageHeaderSize 24
#elif OPENGAUSS
#define HeapPageHeaderSize 40
#endif

#define TYPEALIGN_DOWN(ALIGNVAL, LEN) \
    (((uintptr_t)(LEN) + ((ALIGNVAL) - 1)) & ~((uintptr_t)((ALIGNVAL) - 1)))

// table_name <----> table_oid <----> HeapPageHeader
// map<table_name, map<vector<table_oid>, vector<pageHeader> > >
//std::vector<HeapPageHeader > heapHeaderArry;
std::map<std::string, std::map<std::vector<string >, vector<HeapPageHeader > > > tableOidHeapHeader;

// 检查磁盘上大于1G的oid文件，将他传入到tableOidHeapHeader这个map里，和表名对应起来，为后续HeapPageHeader获取结构体做准备
int checkTableSize(string, vector<string>);

int tableOnDiskOpen(string&, int);

int tableOnDiskTryOpen(int, int);
int tableOnDiskClose(int, int);
int tableOnDiskStates(int, int);

int getTableNameFromDatabase(const string& DBName, vector<string>&, const string&);
int getTableOID(vector<string>& tabList, vector<string>& tabOidList, const string&, const string&, const string&);
int getTableOID(vector<string>, vector<int>&);
int checkTableOIDOnDisk(vector<string>&, string&, string&, vector<string>, vector<string >&);

int getTableConstruct(const char* tableNmae);



// 全局同步变量
std::mutex mtx;
uint32_t tableNodeId = 0; // 表的filenode
uint32_t tableObjectID = 0; // 表的oid
string tableRelName;
vector<tableStructData> tableStructVec;
map<string, vector<tableConstructData>> tableStructMap;

int processRecover();



int processHeapHeaderArry(std::map<std::string, std::map<std::vector<string >, std::vector<HeapPageHeader> > >, vector<string>);
int handleHeapHeader(int, string, string);
char byteToHex(char&);
int InitTable(vector<string>&, const string&, const string&);
int getDBDataBaseDirectory(string&);
uint16_t allocHeaderStruct(vector<uint8_t>&, string, int, uint32_t, string, vector<HeapPageHeader >&);
int saveHeapToHeaderStruct(vector<char>&, string);

int processLargeTableHeapHeaderArry(vector<string>&, map<string , vector<string> >&);

int handleLargeTableHeapHeader(vector<string>&, int);

int appendToGlobalMap(string, string, vector<HeapPageHeader >&);
string vectorCharConvertToString(vector<char>, int, int);

int processDataRecover(vector<string>&, map<string , vector<string> >&, vector<string >&);
int handleHeapDataRecover(int, size_t);
uint16_t resolveHeaderStruct(vector<uint8_t>&, const string&, const string&, int);
int parserHeapTupleData(HeapPageHeader, vector<uint8_t>&, const string&);
int resolveTupleData(vector<vector<uint8_t> >&, HeapPageHeader);
int resolvePgSysTableHeapData(vector<vector<uint8_t> >&, HeapPageHeader);
int resolvePgClassTupeData(vector<uint8_t>&);
int resolvePgAttributeTupeData(vector<uint8_t>&);
string convertToHexString(const std::vector<uint8_t>&);

int findTableStruct(string&, const string&);
int generateCreateTableStatement();

int generateFinalData(vector<uint8_t >&, finalTableData*, ItemIdData);

int displayTupleData(const vector<finalTableData*>&);



#endif //MYSQL_REPLICATER_TABLE_H

