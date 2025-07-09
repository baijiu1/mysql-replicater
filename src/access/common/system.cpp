//
// Created by 白杰 on 2024/12/15.
//

#include "system.h"
#include <libpq-fe.h>
#include <sys/stat.h>
#include <filesystem>
#include <iostream>
#include <map>

using namespace std;
using namespace filesystem;

string concatOidFilePath(string DBDIRPath) {
    string OidDBPath;
    // DBDIRPath = databaseDIR + "/" + tabOidList[i];
    size_t lastPos = DBDIRPath.find_last_of('/');

    if (lastPos != std::string::npos) {
        // DBPath: oid
        OidDBPath = DBDIRPath.substr(0, lastPos);
    }
    return OidDBPath;
};

string getTableOid(string tabOidList) {
    // tabOidList[i]: base/xxx/xx
    string oid;
    size_t lastPos = tabOidList.find_last_of('/');
    if (lastPos != std::string::npos) {
        // DBPath: oid
        oid = tabOidList.substr(lastPos + 1, 30);
    }
    return oid;
};

/*
 * 平衡大表和小表
 * 大表用largeTableList数组，小表用smallTables
 * 小表一张表一个线程去读取
 * 大表分表文件，一个文件一个线程
 */
int balanceTableSizeGroup(vector<string>& tabOidList, string& DBDIRPath, vector<std::vector<string > >& result, int& groupSize, size_t& totalGroups, string DBPath, vector<vector<string > >& largeTableList, map<string , vector<string> >& largeTables) {
    if (tabOidList.size() <= 5) {
        memcpy(&result, &tabOidList, sizeof (tabOidList.size()));
    }

    // 将数组分为两部分
//    std::vector<string> largeTables;
    std::vector<string> smallTables;

    // 平衡数组里的oid，将大小表均匀分布起来
    struct stat info;
    int tableFileSize;
    for (int i = 0; i < tabOidList.size(); ++i) {
        DBDIRPath += tabOidList[i];
        stat(DBDIRPath.c_str(), &info);
        tableFileSize = info.st_size;
        if (tableFileSize >= fileGBsize) {
            // 再次去磁盘上查看有多少个数据文件
            // map存储
            for (const auto& entry : directory_iterator(DBPath)) {
                const auto& file_path = entry.path();
                if (file_path.filename().string().find(tabOidList[i]) == 0) {
                    std::cout << "Found file: " << file_path << std::endl;
                    largeTables[tabOidList[i]].push_back(file_path);
                }
            }
//            largeTables.push_back(DBDIRPath);
//            largeTableList[i].push_back(DBDIRPath);
        } else {
            smallTables.push_back(DBDIRPath);
        }
    }
    // 将tables分割成每5个为一组
    for (size_t i = 0; i < smallTables.size(); i += groupSize) {
        std::vector<string > group;
        for (size_t j = i; j < i + groupSize && j < smallTables.size(); ++j) {
            group.push_back(smallTables[j]);
        }
        result.push_back(group);
    }
    // 将大数值均匀分布到各组中
//    size_t largeIndex = 0;
//    size_t largeCount = largeTables.size();
//    for (size_t i = 0; i < totalGroups; ++i) {
//        if (largeIndex < largeCount) {
//            result[i].push_back(largeTables[largeIndex++]);
//        }
//    }
    // 将小数值补充到组中
//    size_t smallIndex = 0;
//    size_t smallCount = smallTables.size();
//    for (size_t i = 0; i < totalGroups; ++i) {
//        while (result[i].size() < groupSize && smallIndex < smallCount) {
//            result[i].push_back(smallTables[smallIndex++]);
//        }
//    }
    // 如果还有剩余的大数值，继续分配
//    for (size_t i = 0; i < totalGroups; ++i) {
//        if (result[i].size() < groupSize && largeIndex < largeCount) {
//            result[i].push_back(largeTables[largeIndex++]);
//        }
//    }
};

// 生成指定字节数的一个vector<uint8_t >数组，用于后面转换小端序。从beginPos开始，到endPos结束的字节
int generateGroup(vector<uint8_t >& group, int beginPos, int endPos, vector<uint8_t>& tuple) {
//    cout << "beginPos: " << beginPos << endl;
//    cout << "endPos: " << endPos << endl;
    if (beginPos < 0 || endPos > tuple.size() || beginPos > endPos) {
        throw std::out_of_range("generateGroup exceeds group size");
        return 0;
    }
    group.clear();
    group.insert(group.end(), tuple.begin() + beginPos, tuple.begin() + endPos);
};


// Helper function to convert 4 bytes to uint32_t (Little Endian)
uint32_t convertBigToLittleEndian32(const std::vector<uint8_t>& vec, size_t offset) {
    if (offset + 4 > vec.size()) {
        return 1;
    }
    return static_cast<uint32_t>(static_cast<unsigned char>(vec[offset])) |
           static_cast<uint32_t>(static_cast<unsigned char>(vec[offset + 1])) << 8 |
           static_cast<uint32_t>(static_cast<unsigned char>(vec[offset + 2])) << 16 |
           static_cast<uint32_t>(static_cast<unsigned char>(vec[offset + 3])) << 24;
}

// Helper function to convert 2 bytes to uint16_t (Little Endian)
uint16_t convertBigToLittleEndian16(const std::vector<uint8_t>& vec, size_t offset) {
    return static_cast<uint16_t>(static_cast<unsigned char>(vec[offset])) |
           static_cast<uint16_t>(static_cast<unsigned char>(vec[offset + 1])) << 8;
}

// Helper function to convert 8 bytes to uint64_t (Little Endian)
uint64_t convertBigToLittleEndian64(const std::vector<uint8_t>& vec, size_t offset) {
    return static_cast<uint64_t>(static_cast<unsigned char>(vec[offset])) |
           static_cast<uint64_t>(static_cast<unsigned char>(vec[offset + 1])) << 8 |
           static_cast<uint64_t>(static_cast<unsigned char>(vec[offset + 2])) << 16 |
           static_cast<uint64_t>(static_cast<unsigned char>(vec[offset + 3])) << 24 |
           static_cast<uint64_t>(static_cast<unsigned char>(vec[offset + 4])) << 32 |
           static_cast<uint64_t>(static_cast<unsigned char>(vec[offset + 5])) << 40 |
           static_cast<uint64_t>(static_cast<unsigned char>(vec[offset + 6])) << 48 |
           static_cast<uint64_t>(static_cast<unsigned char>(vec[offset + 7])) << 56;
}



size_t eliminateSlash(vector<string>& tabOidList, string& DBPath) {
    size_t lastSlashPos = tabOidList[0].find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        // 截取从头到倒数第二个斜杠为止的部分
        // DBPath: /Users/haruhi/ClionProjects/postgresql-17.2/pg_install/data/base/16384
//        DBPath = tabOidList[0].substr(0, lastSlashPos);
        DBPath = tabOidList[0].substr(lastSlashPos + 1, 30);
    }
    return lastSlashPos;
};

size_t eliminateSlashString(string& tabOidList, string& DBPath) {
    size_t lastSlashPos = tabOidList.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        // DBPath: oid
        DBPath = tabOidList.substr(lastSlashPos + 1, 30);
    }
    return lastSlashPos;
};