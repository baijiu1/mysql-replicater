//
// Created by 白杰 on 2024/12/15.
//

#ifndef MYSQL_REPLICATER_SYSTEM_H
#define MYSQL_REPLICATER_SYSTEM_H
#include <string>
#include <iostream>
#include <map>
#include <vector>



using namespace std;
#define fileGBsize 1073741824


int balanceTableSizeGroup(vector<string>&, string&, vector<std::vector<string > >&, int&, size_t&, string, vector<vector<string > > &, map<string , vector<string> >&);
uint16_t convertBigToLittleEndian16(const std::vector<uint8_t>& vec, size_t );
uint32_t convertBigToLittleEndian32(const std::vector<uint8_t>& vec, size_t );
uint64_t convertBigToLittleEndian64(const std::vector<uint8_t>& vec, size_t );

// 消除斜杠
size_t eliminateSlash(vector<string>&, string& );
size_t eliminateSlashString(string&, string& );

// 拼接数据库目录到/Users/haruhi/ClionProjects/postgresql-17.2/pg_install/data/base/16384/
string concatOidFilePath(string);
string getTableOid(string );

int generateGroup(vector<uint8_t >&, int, int, vector<uint8_t>&);


#endif //MYSQL_REPLICATER_SYSTEM_H
