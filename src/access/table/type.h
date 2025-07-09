//
// Created by 白杰 on 2024/12/29.
//

#ifndef MYSQL_REPLICATER_TYPE_H
#define MYSQL_REPLICATER_TYPE_H

#include <string>
#include "page.h"
//#include "event.h"
#include <map>

using namespace std;

//extern struct tableStructData;
//extern tableStructData* tableStruct;
typedef int (*typeEvent)(vector<uint8_t>&, vector<uint8_t>&, uint32_t *, string);

extern map<string, typeEvent> typeEventFuncMap;
extern int InitEventFuncMap();

string getColumnTypeName(uint32_t);
uint32_t getTypeStoreByteCount1(vector<tableConstructData >&, int, vector<uint8_t >&, uint32_t *);
uint32_t getTypeStoreByteCount(vector<tableStructData >&, int, vector<uint8_t >&);



#endif //MYSQL_REPLICATER_TYPE_H
