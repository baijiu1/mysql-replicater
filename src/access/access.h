//
// Created by 白杰 on 2024/12/17.
//

#ifndef MYSQL_REPLICATER_ACCESS_H
#define MYSQL_REPLICATER_ACCESS_H
#include <string>
#include <map>
#include <vector>
//#include "table/table.h"

using namespace std;

extern int InitTable(vector<string>&, const string&, const string&);

int InitAccess();
int InitAccessForFile(const string&);

int InitAccessForDBAndTableName(const string&, const string&);

int InitAccessForCParserFile(string);




#endif //MYSQL_REPLICATER_ACCESS_H
