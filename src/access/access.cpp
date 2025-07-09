//
// Created by 白杰 on 2024/12/17.
// 拿到header的全局map后，根据pd_linp的偏移量，取具体的元组
// 元组中计算该元组是否是老版本
//

#include "access.h"
#include <vector>
#include "string"

using namespace std;


//int InitAccess() {
//    vector<string> tabList;
//    InitTable(tabList);
//
//};

int InitAccessForFile(const string& fileName) {
    vector<string> tabList;
//    InitTable(tabList, fileName, "fromFIleOID");
}

int InitAccessForDBAndTableName(const string& dbName, const string& tableName) {
    vector<string> tabList;
    tabList.clear();
//    InitTable(tabList, dbName, tableName);
    InitTable(tabList, dbName, tableName);
}

