//
// Created by 白杰 on 2024/12/14.
//

#include "table.h"
#include "page.h"
#include <utility>
#include <vector>
#include <string>
#include <libpq-fe.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <filesystem>
#include <numeric>
#include <sys/mman.h>
#include <unistd.h>
#include <thread>
#include <iomanip>
#include <mutex>



using namespace std;
using namespace filesystem;


/*
 * get dbname from input args
 */
int getTableNameFromDatabase(const string& DBName, vector<string>& tabList, const string& tableName) {
    const char* conninfo = "host=localhost port=15432 dbname=test user=haruhi password=123456";

    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        printf("connect database failed");
        PQfinish(conn);
        return 1;
    }

    char queryStr[520];
    snprintf(queryStr, sizeof(queryStr), "SELECT table_name FROM information_schema.tables "
                                         "WHERE table_type= 'BASE TABLE' "
                                         "AND table_schema NOT IN ('information_schema', 'pg_catalog') AND "
                                         "table_catalog = '%s';", DBName.c_str());

    PGresult* res = PQexec(conn, queryStr);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("Error executing query: %s ", PQerrorMessage(conn));
        PQclear(res);
        return 1;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
//        printf(" %s ", PQgetvalue(res, i, 0));
        if (PQgetvalue(res, i, 0) == tableName) {
            tabList.emplace_back(PQgetvalue(res, i, 0));
        }

    }
    if (tabList.empty()) {
        printf("cannot match give table name in %s.", DBName.c_str());
        return 1;
    }


    PQclear(res);
    return 0;
}

/*
 * get table oid from pg_class
 */
int getPGClassInfoWithTable(const string& DBName, vector<string>& tabList, const string& tableName) {
    const char* conninfo = "host=localhost port=15432 dbname=test user=haruhi password=123456";

    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        printf("connect database failed");
        PQfinish(conn);
        return 1;
    }

    char queryStr[520];
    snprintf(queryStr, sizeof(queryStr), "SELECT oid FROM pg_class "
                                         "WHERE relname = '%s' ", tableName.c_str());

    PGresult* res = PQexec(conn, queryStr);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("Error executing query: %s ", PQerrorMessage(conn));
        PQclear(res);
        return 1;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
//        printf(" %s ", PQgetvalue(res, i, 0));
        if (PQgetvalue(res, i, 0) == tableName) {
            tabList.emplace_back(PQgetvalue(res, i, 0));
        }

    }
    if (tabList.empty()) {
        printf("cannot match give table name in %s.", DBName.c_str());
        return 1;
    }


    PQclear(res);
    return 0;
}

/*
 * get table attribute from pg_attribute
 */
int getPGAttributeWithTable(const string& tableName) {
    const char* conninfo = "host=localhost port=15432 dbname=test user=haruhi password=123456";

    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        printf("connect database failed");
        PQfinish(conn);
        return 1;
    }

    char queryStr[720];
    snprintf(queryStr, sizeof(queryStr), "SELECT attrelid, class.relname, attname, typ.typname, attlen, attnum, atttypmod, "
                                         "attstorage, attcompression, attnotnull, atthasdef, attisdropped, "
                                         "attislocal, attinhcount, attcollation, "
                                         "(SELECT setting FROM pg_settings WHERE name = 'data_directory') || '/' || pg_relation_filepath(class.oid) as relfilenode,"
                                         "pg_relation_size(relname::regclass) AS size_pretty, class.relfilenode, attr.atttypid, attr.attalign "
                                         "FROM pg_attribute attr "
                                         "left join pg_class class on attr.attrelid = class.oid LEFT JOIN "
                                         "pg_type typ ON attr.atttypid = typ.oid "
                                         "where class.relname = '%s' and attr.attnum >= 1;", tableName.c_str());

    PGresult* res = PQexec(conn, queryStr);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("Error executing query: %s ", PQerrorMessage(conn));
        PQclear(res);
        return 1;
    }

    int rows = PQntuples(res);
    vector<tableConstructData> tableStructVecData;
    for (int i = 0; i < rows; ++i) {
        tableConstructData tableStructVar;
        tableStructVar.oid = PQgetvalue(res, i, 0);
        tableStructVar.tableName = PQgetvalue(res, i, 1);
        if ( i == rows - 1) {
            tableRelName = tableStructVar.tableName;
        }
        tableStructVar.columnName = PQgetvalue(res, i, 2);
        tableStructVar.columnTypeName = PQgetvalue(res, i, 3);
        tableStructVar.columnLength = PQgetvalue(res, i, 4);
        tableStructVar.columnNum = PQgetvalue(res, i, 5);
        tableStructVar.columnTypeMod = PQgetvalue(res, i, 6);
        tableStructVar.columnStorage = PQgetvalue(res, i, 7);
        tableStructVar.columnCompression = PQgetvalue(res, i, 8);
        tableStructVar.columnNotNull = PQgetvalue(res, i, 9);
        tableStructVar.columnHasDefault = PQgetvalue(res, i, 10);
        tableStructVar.columnIsdropped = PQgetvalue(res, i, 11);
        tableStructVar.columnIslocal = PQgetvalue(res, i, 12);
        tableStructVar.columnInhcount = PQgetvalue(res, i, 13);
        tableStructVar.columnCollation = PQgetvalue(res, i, 14);
        tableStructVar.filenodePath = PQgetvalue(res, i, 15);
        tableStructVar.tableSize = PQgetvalue(res, i, 16);
        tableStructVar.relFilenode = PQgetvalue(res, i, 17);
        tableStructVar.columnTypeId = PQgetvalue(res, i, 18);
        tableStructVar.columnAttalign = PQgetvalue(res, i, 19);
        tableStructVecData.push_back(tableStructVar);
    }
    tableStructMap[tableRelName] = tableStructVecData;

    if (tableStructMap[tableRelName].empty()) {
        throw runtime_error("cannot get give table in pg_attribute from database");
    }
    PQclear(res);
    return 0;
}


int getTableOID(vector<string>& tabList, vector<string>& tabOidList, const string& DBName, const string& userName, const string& passwd) {
    // SELECT
    //    pg_total_relation_size('your_table_name') AS total_size_bytes,
    //    pg_total_relation_size('your_table_name') / (1024 * 1024 * 1024) AS num_segments;
    // 拿到数据文件有多少个段，然后再填充进去
    const char* conninfo = "host=localhost port=15432 dbname=test user=haruhi password=123456";
//    char  conninfo[520];
//    snprintf(conninfo, sizeof(conninfo), "host=localhost port=5432 dbname=%s user=%s password=%s;", DBName.c_str(), userName.c_str(), passwd.c_str());

    char queryStr[520];
    for (string tab : tabList) {
        PGconn* conn = PQconnectdb(conninfo);
        if (PQstatus(conn) != CONNECTION_OK) {
            printf("connect database failed");
            PQfinish(conn);
            return 1;
        }
        snprintf(queryStr, sizeof(queryStr), "SELECT pg_relation_filepath('%s');", tab.c_str());
        PGresult* res = PQexec(conn, queryStr);
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            printf("Error executing query: %s ", PQerrorMessage(conn));
            PQclear(res);
            return 1;
        }

        int rows = PQntuples(res);
        for (int i = 0; i < rows; ++i) {
            tabOidList.emplace_back(PQgetvalue(res, i, 0));
        }
        PQclear(res);
    }
    return 0;
}

/*
 * get data directory path for open table data file
 * SHOW data_directory;
 */
int getDBDataBaseDirectory(string& DBDIRPath) {
    const char* conninfo = "host=localhost port=15432 dbname=test user=haruhi password=123456";


    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        printf("connect database failed");
        PQfinish(conn);
        return 1;
    }

    char queryStr[520];
    snprintf(queryStr, sizeof(queryStr), "SHOW data_directory;");
    PGresult* res = PQexec(conn, queryStr);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        printf("Error executing query: %s ", PQerrorMessage(conn));
        PQclear(res);
        return 1;
    }

    int rows = PQntuples(res);
    // DBDIRPath：/Users/haruhi/ClionProjects/postgresql-17.2/pg_install/data
    for (int i = 0; i < rows; ++i) {
        DBDIRPath = PQgetvalue(res, i, 0);
    }


    PQclear(res);
    return 0;
}


/*
 * check table data file exists on disk
 * 拼接全路径，为后面open做准备
 * 切割到具体到oid
 * 判断大于1G的表，将oid oid.1 oid.2放到全局的mao里，和表名对应
 */
int checkTableOIDOnDisk(vector<string>& tabOidList, string& DBPath, string& DBDIRPath, vector<string> tableList, vector<string >& oidGBList) {
    // split '/' in vector of tabOidList
    // tabOidList: base/xxxx/xxxx
    // DBDIRPath：/Users/haruhi/ClionProjects/postgresql-17.2/pg_install/data
    getDBDataBaseDirectory(DBDIRPath);
    string databaseDIR = DBDIRPath;
    for (int i = 0; i < tabOidList.size(); ++i) {
        struct stat info;
        DBDIRPath = databaseDIR + "/" + tabOidList[i];
        stat(DBDIRPath.c_str(), &info);
        oidGBList.clear();
        int tableFileSize = info.st_size;
        if (tableFileSize >= fileGBsize) {
            // 再次去磁盘上查看有多少个数据文件
            // map存储
            for (const auto& entry : directory_iterator(concatOidFilePath(DBDIRPath))) {
                string file_path = entry.path().filename().string();
                if (file_path.rfind(getTableOid(tabOidList[i]), 0) == 0) {
                    // 包含了fsm和vm文件，排除
                    if (file_path.ends_with("_fsm") || file_path.ends_with("_vm")) {
                        continue;
                    }
                    file_path = concatOidFilePath(DBDIRPath) + "/" + file_path;
                    oidGBList.emplace_back(file_path);
                }
            }
        } else {
            // 就一个数据文件的
            oidGBList.emplace_back(DBDIRPath.c_str());
        }
        tableOidHeapHeader[tableList[i]][oidGBList].push_back(new HeapPageHeaderData);
        delete tableOidHeapHeader[tableList[i]][oidGBList][0] ;
        tableOidHeapHeader[tableList[i]][oidGBList].erase(tableOidHeapHeader[tableList[i]][oidGBList].begin());
        DBDIRPath.clear();
        // 假设 tableOidHeapHeader 是外层 map，oidGBList 是内层 map 的键
//        for (const auto& tableEntry : tableOidHeapHeader) {
//            const auto& tableKey = tableEntry.first;   // tableList[i] 的键
//            const auto& tableValue = tableEntry.second; // 对应的内层 map
//
//            std::cout << "tableKey: " << tableKey << std::endl;
//
//            // 遍历内层 map
//            for (const auto& oidEntry : tableValue) {
//                const auto& oidKey = oidEntry.first; // oidGBList 的键
//                const auto& oidValue = oidEntry.second; // 该键对应的 vector<HeapPageHeaderData>
//
//                for (int j = 0; j < oidKey.size(); ++j) {
//                    printf("oidKey: %s \n", oidKey[j].c_str());
//                }
//                std::cout << "oidValue size: " << oidValue.size() << std::endl;
//            }
//        }
    }
    return 1;
}

/*
 * open table from disk
 */
int tableOnDiskOpen(string& DBFilePath, int lockMode) {

    int fd = open(DBFilePath.c_str(), O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }
    return fd;
}

int tableOnDiskStates(int fd, int lockMode) {
    // 检查文件描述符状态
    if (fd >= 0) {
//        std::cout << "File descriptor " << fd << " is valid." << std::endl;
        return 1;
    } else {
//        std::cout << "File descriptor " << fd << " is invalid." << std::endl;
        return fd;
    }
}

int tableOnDiskClose(int fd, int lockMode) {
    return close(fd);
}

/*
 * 填充heap header结构体
 * ----------------------------------------------------------------------------------------------------------------------|
 * | pd_lsn	 8byte        |   pd_checksum 2byte | pd_flags 2byte |  pd_lower 2byte	 | pd_upper 2byte | pd_special 2byte |
   ----------------------------------------------------------------------------------------------------------------------|
   | pd_pagesize_version 2byte |	pd_prune_xid 4byte | pd_xid_base 8byte | pd_multi_base 8byte | pd_linp[0] 4byte      |
   ----------------------------------------------------------------------------------------------------------------------|

 */
uint16_t allocHeaderStruct(vector<uint8_t>& pageHeader, string tableOID, int fd, uint32_t addr, string tableName, vector<HeapPageHeader >& heapHeaderArry) {
//    lock_guard<std::mutex> lock(mtx);

    HeapPageHeader HeapHeader = new HeapPageHeaderData;
    HeapHeader->pd_lsn.xlogid = convertBigToLittleEndian32(pageHeader, 0);
    HeapHeader->pd_lsn.xrecoff = convertBigToLittleEndian32(pageHeader, 4);
    HeapHeader->pd_checksum = convertBigToLittleEndian16(pageHeader, 8);
    HeapHeader->pd_flags = convertBigToLittleEndian16(pageHeader, 10);
    HeapHeader->pd_lower = convertBigToLittleEndian16(pageHeader, 12);
    HeapHeader->pd_upper = convertBigToLittleEndian16(pageHeader, 14);
    HeapHeader->pd_special = convertBigToLittleEndian16(pageHeader, 16);
    HeapHeader->pd_pagesize_version = convertBigToLittleEndian16(pageHeader, 18);
    HeapHeader->pd_prune_xid = convertBigToLittleEndian32(pageHeader, 20);

#ifdef OPENGAUSS
    HeapHeader->pd_xid_base = convertBigToLittleEndian16(stoull(accumulate(pageHeader.begin() + 24, pageHeader.end() + 32, std::string()), nullptr, 16));
    HeapHeader->pd_multi_base = convertBigToLittleEndian16(stoull(accumulate(pageHeader.begin() + 32, pageHeader.end() + 40, std::string()), nullptr, 16));
#endif

#ifdef __APPLE__
    // 传入两次，一次是16K，一次是8K
    // 对应是前8K和后8K

    if (pageHeader.size() >= 16384) {
        vector<uint8_t> logicPage1PDLinp(pageHeader.begin(), pageHeader.begin() + HeapHeader->pd_lower - HeapPageHeaderSize);
//        memcpy(&logicPage1PDLinp, &pageHeader, addr + HeapHeader->pd_lower - HeapPageHeaderSize);
        // first page
        int k = 0;
        for (size_t i = 0; i < logicPage1PDLinp.size(); i += 4) {
//            printf(" %x ", logicPage1PDLinp[i]);
            std::vector<uint8_t> group;
            for (size_t j = i; j < i + 4 && j < logicPage1PDLinp.size(); ++j) {
                group.push_back(logicPage1PDLinp[j]);
            }
            // 转换二进制
            uint32_t pdlinpArry = convertBigToLittleEndian32(group, 0);
            bitset<32> pdLinp(pdlinpArry);
//            HeapHeader->pd_linp[k]->lp_len = pdlinpArry >> 17;
//            HeapHeader->pd_linp[k]->lp_flags = (pdlinpArry >> 1) & 1;
//            HeapHeader->pd_linp[k]->lp_off = (pdlinpArry & 0x7FFF);
            k++;
        }
        heapHeaderArry.push_back(HeapHeader);

        logicPage1PDLinp.clear();
    } else if (pageHeader.size() <= 8194) {
        // second page
        vector<uint8_t> logicPage2PDLinp(pageHeader.begin(), pageHeader.begin() + 8192 + HeapHeader->pd_lower - HeapPageHeaderSize);
//        memcpy(logicPage2PDLinp.data(), pageHeader.data(),addr + 8192 + HeapHeader->pd_lower - HeapPageHeaderSize);
        int k = 0;
        for (size_t i = 0; i < logicPage2PDLinp.size(); i += 4) {
            std::vector<uint8_t> group;
            for (size_t j = i; j < i + 4 && j < logicPage2PDLinp.size(); ++j) {
                group.push_back(logicPage2PDLinp[j]);
            }
            // 转换二进制
            uint32_t pdlinpArry = convertBigToLittleEndian32(group, 0);
            bitset<32> pdLinp(pdlinpArry);
//            HeapHeader->pd_linp[k]->lp_len = pdlinpArry >> 17;
//            HeapHeader->pd_linp[k]->lp_flags = (pdlinpArry >> 1) & 1;
//            HeapHeader->pd_linp[k]->lp_off = (pdlinpArry & 0x7FFF);
            k++;
        }
        heapHeaderArry.push_back(HeapHeader);
        logicPage2PDLinp.clear();
    }
    delete HeapHeader;
#elif defined(__linux__)
    vector<uint8_t> logicPage1PDLinp(pageHeader.begin(), pageHeader.begin() + HeapHeader->pd_lower - HeapPageHeaderSize);
    int k = 0;
    for (size_t i = 0; i < logicPage1PDLinp.size(); i += 4) {
        std::vector<uint8_t> group;
        for (size_t j = i; j < i + 4 && j < logicPage1PDLinp.size(); ++j) {
            group.push_back(logicPage1PDLinp[j]);
        }
        // 转换二进制
        uint32_t pdlinpArry = convertBigToLittleEndian32(group, 0);
        bitset<32> pdLinp(pdlinpArry);
        HeapHeader->pd_linp[k].lp_len = pdlinpArry >> 17;
        HeapHeader->pd_linp[k].lp_flags = (pdlinpArry >> 1) & 1;
        HeapHeader->pd_linp[k].lp_off = (pdlinpArry & 0x7FFF);
        k++;
    }
    heapHeaderArry.push_back(HeapHeader);
    logicPage1PDLinp.clear();
    delete HeapHeader;
#endif
    return 0;
}

int appendToGlobalMap(string tableOID, string tableName, vector<HeapPageHeader >& heapHeaderArry) {
    lock_guard<std::mutex> lock(mtx);
    vector<string > oidKey;
    oidKey.clear();
    auto tableIt = tableOidHeapHeader.find(tableName);
    if (tableIt == tableOidHeapHeader.end()) {
        // tableOidHeapHeader里没有tableName这个元素
        oidKey.push_back(tableOID);
        tableOidHeapHeader[tableName][oidKey] = heapHeaderArry;
    } else {
        // tableOidHeapHeader里有tableName这个元素,说明这是个大于1G的表，有多个oid数据文件
        // 先把oid原有的拿出来，然后append到oidKey里
        for (const auto& oidListPair : tableOidHeapHeader[tableName]) {
            for (string oid: oidListPair.first) {
                oidKey.push_back(oid);
            }
        }
        oidKey.push_back(tableOID);
        tableOidHeapHeader[tableName][oidKey] = heapHeaderArry;
    }
    return 0;
    // 打印tableOidHeapHeader的内容
//    for (const auto& tableEntry : tableOidHeapHeader) {
//        const auto& tableKey = tableEntry.first;   // tableList[i] 的键
//        const auto& tableValue = tableEntry.second; // 对应的内层 map
//
//        // 遍历内层 map
//        for (const auto& oidEntry : tableValue) {
//            const auto& oidKey = oidEntry.first; // oidGBList 的键
//            const auto& oidValue = oidEntry.second; // 该键对应的 vector<HeapPageHeaderData>
//
//            std::cout << "oidValue size: " << oidValue.size() << std::endl;
//        }
//    }
}

int handleHeapHeader(int fd, string tableOID, string tableName) {
    // 循环读取0x2000 0x4000 ... 的header，固定24字节，获取到pd_lower和pd_upper，然后根据
    // pd_lower获取到pd_linp的数组
    // tableOID:/Users/haruhi/ClionProjects/postgresql-17.2/pg_install/data/base/16384/49287
//    lock_guard<std::mutex> lock(mtx);

#ifdef __APPLE__
    const uint32_t darwinStart = 0x0000;       // 起始地址
    const uint32_t darwinEnd = 0x40000000;    // 结束地址
    const uint32_t darwinStep = 0x4000;       // 每页大小 16K
    // apple os use huge page, 16K
    printf(" begin mmap: %s \n", tableOID.c_str());
    std::vector<HeapPageHeader > heapHeaderArry;
    for (uint32_t addr = darwinStart; addr < darwinEnd; addr += darwinStep) {
        printf(" addr: %d ", addr);
        // 如果文件小于16K
        if (addr >= lseek(fd, 0, SEEK_END)) {
            return 1;
        }
        void* PageHeaderDATAAPPLE = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, addr);
        if (PageHeaderDATAAPPLE == MAP_FAILED) {
            perror("mmap failed");
            close(fd);
            return 1;
        }
        // logical_page1指向16K前8K的头
        // logical_page2指向16K后8K的头
        vector<uint8_t > logical_page1(_PAGESIZE + 1);
        vector<uint8_t > logical_page2(8192 + 1);
        logical_page1.reserve(_PAGESIZE + 1);
        logical_page2.reserve(_PAGESIZE / 2);
        memcpy(logical_page1.data(), PageHeaderDATAAPPLE, _PAGESIZE);
        memcpy(logical_page2.data(), logical_page1.data() + 8192, 8192);
        munmap(PageHeaderDATAAPPLE, _PAGESIZE);

        allocHeaderStruct(logical_page1, tableOID, fd, addr, tableName, heapHeaderArry);
        logical_page1.clear();
        logical_page2.clear();
    }
    printf(" end mmap, begin appen to global map for every table ");
    appendToGlobalMap(tableOID, tableName, heapHeaderArry);
//    heapHeaderArry.clear();
#elif defined(__linux__)
    const uint32_t start = 0x0000;       // 起始地址
    const uint32_t end = 0x40000000;    // 结束地址
    const uint32_t step = 0x2000;       // 每页大小 8K
    string pdUpper, pdLower;
    printf("\nbegin mmap: %s \n", tableOID.c_str());
    for (uint32_t addr = start; addr < end; addr += step) {
        printf("\n handleHeapHeader addr: %d fd: %d HeapPageHeaderSize: %d \n", addr, fd, HeapPageHeaderSize);
        void* PageHeaderDATA = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, addr);
        if (PageHeaderDATA == MAP_FAILED) {
            perror("PageHeaderDATA mmap failed.");
            close(fd);
            return 1;
        }
        // _PAGESIZE 4K大小 每次映射
        vector<uint8_t > logical_page1(_PAGESIZE + 1);
        logical_page1.reserve(_PAGESIZE + 1);
        memcpy(logical_page1.data(), PageHeaderDATA, _PAGESIZE);
        munmap(PageHeaderDATA, _PAGESIZE);
        if (pageHeader[15] == 0 && pageHeader[16] == 0 ) {
            // 没数据
            printf("read mmap context empty. return...");
            return 1;
        }
        // 分配header到结构体的成员
        allocHeaderStruct(logical_page1, tableOID, fd, addr, tableName);
    }
    printf(" end mmap, begin appen to global map for every table ");
    appendToGlobalMap(tableOID, tableName);
//    heapHeaderArry.clear();
#endif
    return 0;
}



int processHeapHeaderArry(std::map<std::string, std::map<std::vector<string >, std::vector<HeapPageHeader> > > tableOidHeapHeader, vector<string> tableList, vector<string> oidGBList) {
    // 处理表，为每张表都开启一个线程去处理
    // 这里是数据同步的部分
    vector<thread> tableHandleThreads;
    vector<thread> smallTableHandleThreads;
    for (int i = 0; i < tableOidHeapHeader.size(); ++i) {
        for (const auto& oidListPair : tableOidHeapHeader[tableList[i]]) {
            if (oidListPair.first.size() == 1) {
                // 小于1G的表，也就是只有一个oid数据文件
                for (string oid: oidListPair.first) {

                    int fd = tableOnDiskOpen(oid, 10);
                    if (fd == -1) {
                        printf("open file failed...\n");
                    }
                    lseek(fd, 0, SEEK_SET); // 重置文件偏移量

                    smallTableHandleThreads.emplace_back(handleHeapHeader, fd, oid, tableList[i]);
                    // wait for internal thread to finish
                    if ((i + 1) % 3 == 0 || i == smallTableHandleThreads.size() - 1) {
                        // 等待所有线程完成
                        for (auto& t : smallTableHandleThreads) {
                            if (t.joinable()) {
                                t.join();
                            }
                        }
                        smallTableHandleThreads.clear();
                    }
                }
            } else {
                // 大于1G的表，多个oid数据文件
                for (string oid: oidListPair.first) {
                    int fd = tableOnDiskOpen(oid, 10);
                    lseek(fd, 0, SEEK_SET); // 重置文件偏移量
//                    handleHeapHeader(fd, oid, tableList[i]);
                    tableHandleThreads.emplace_back(handleHeapHeader, fd, oid, tableList[i]);
                    // wait for internal thread to finish
                    if ((i + 1) % 3 == 0 || i == tableOidHeapHeader.size() - 1) {
                        // 等待所有线程完成
                        for (auto& t : tableHandleThreads) {
                            if (t.joinable()) {
                                t.join();
                            }
                        }
                        tableHandleThreads.clear();
                    }
                    tableOnDiskClose(fd, 100);
                }
            }
        }
    }


    return 0;
}

/*
 * we produce final data of tuple. according to column type of table struct(tableStructVec) to parser user data
 */
int generateFinalData(vector<uint8_t >& group, finalTableData* tableNode, ItemIdData pdLinp) {
    // 起始地址
    uint32_t *lpOff = new uint32_t;
    *lpOff = pdLinp.lp_off + 24;
    for (int i = 0; i < tableStructMap[tableRelName].size(); ++i) {
        tableNode->t_data.push_back(to_string(i));
//        printf("\n I: %u , groupSize: %zu \n", i, group.size());
        getTypeStoreByteCount1(tableStructMap[tableRelName], i, group, lpOff);
    }
    delete lpOff;
};

/*
 * @param
 * @tuple tuple data from heap
 * resolve tuple data in heap
 * if (t_hoff > 24) {
 * -------------------------------------------
 * t_len   t_self   t_tableOid   t_data
 * -------------------------------------------
 *
 * -------------------------------------------
 * } else if (t_hoff == 24) {
 * -------------------------------------------
 * t_data
 * -------------------------------------------
 *
 * }
 * t_data is tuple header + tuple data:
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------
 * t_xmin(uint32) t_xmax(uint32) t_cid(uint32) t_ctid(uint32 + (uint16) t_infomask2(uint16) t_infomask(uint16) t_hoff(uint8) t_bits(uint8) data(pd_linp->lp_len)
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------
 *
 */
int resolveTupleData(vector<vector<uint8_t> >& tuple, HeapPageHeader HeapHeader) {
//    HeapTuple tupleData = new HeapTupleData;
//    finalTableData* tableNode = (finalTableData*)malloc(sizeof(finalTableData));
    vector<finalTableData*> displayData;
    for (int i = 0; i < tuple.size(); ++i) {
        finalTableData* tableNode = new finalTableData();
        // 判断t_hoff
        if (tuple[i][22] == 24) {
            vector<uint8_t > group;
            // xmin
            generateGroup(group, 0, 4, tuple[i]);
            uint32_t tXMIN= convertBigToLittleEndian32(group, 0);
            group.resize(0);
            // xmax 如果是可见的数据，这里应该为0。如果有值，那么指向xmin，指向的xmin就是最新版本的数据，有值就是老版本的数据
            generateGroup(group, 4, 8, tuple[i]);
            uint32_t tXMAX = convertBigToLittleEndian32(group, 0);
            group.resize(0);
            tableNode->t_xmin = tXMIN;
            tableNode->t_xmax = tXMAX;
            // cid
            generateGroup(group, 8, 12, tuple[i]);
            uint32_t tCID = convertBigToLittleEndian32(group, 0);
            group.resize(0);
            // （页号，偏移量） 以下条件可以认为是该字段没有更改过，可以直接显示
            // ctid 的 block ID == 当前页号
            // ctid 的 offset ID == 当前元组在页中的 offset
            // ctid -> blockID
            generateGroup(group, 12, 16, tuple[i]);
            uint32_t tCTID_blockID = convertBigToLittleEndian32(group, 0);
            printf("\ntCTID_blockID: %u\n", tCTID_blockID);
            group.resize(0);
            // ctid -> offsetNumber
            generateGroup(group, 16, 18, tuple[i]);
            uint16_t tCTID_offsetNumer = convertBigToLittleEndian16(group, 0);
            printf("\ntCTID_offsetNumer: %u\n", tCTID_offsetNumer);
            group.resize(0);
            // t_infomask2
            generateGroup(group, 18, 20, tuple[i]);
            uint32_t tInfo2 = convertBigToLittleEndian32(group, 0);
            group.resize(0);
            // t_infomask
            generateGroup(group, 20, 22, tuple[i]);
            uint32_t tInfo = convertBigToLittleEndian32(group, 0);
            group.resize(0);
            // t_hoff
            generateGroup(group, 22, 23, tuple[i]);
            uint32_t tHoff = convertBigToLittleEndian32(group, 0);
            group.resize(0);
            // t_bits
            generateGroup(group, 23, 24, tuple[i]);
            uint32_t tBits = convertBigToLittleEndian32(group, 0);
            group.resize(0);
            // t_data
            generateGroup(group, 24, HeapHeader->pd_linp[i].lp_len, tuple[i]);
            // 根据类型来判断这是什么规则，比如int就是直接4个字节，varchar或者char需要长度+内容
            generateFinalData(group, tableNode, HeapHeader->pd_linp[i]);
//            printf(" t_data \n");
//            for (int j = 0; j < group.size(); ++j) {
//                printf(" %x ", group[j]);
//            }
//            printf("\n");
            displayData.push_back(tableNode);

        } else {

        }

    }
    // 最后再做链表
    for (auto& o : displayData) {
        if (o->t_xmax != 0) {
            for (auto& n : displayData) {
                if (o->t_xmax == n->t_xmin) {
                    o->next = n;
                }
            }
        }
    }
    // 显示具体数据
    displayTupleData(displayData);
    // 使用完 displayData 后，手动释放内存
    for (auto ptr : displayData) {
        delete ptr;
    }


}

int displayTupleData(const vector<finalTableData*>&) {

};

string convertToHexString(const std::vector<uint8_t>& data) {
    std::ostringstream hexStream;

    for (uint8_t byte : data) {
        // 转换为两位十六进制格式
        hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }

    return hexStream.str();
}

/*
 * 解析pg_class表内容
 *
 */
int resolvePgClassTupeData(vector<uint8_t>& tuple) {
    Form_pg_class pgClassData = new FormData_pg_class;
    vector<uint8_t > group;
    generateGroup(group, 0, 4, tuple);
    uint32_t oid = convertBigToLittleEndian32(group, 0);
    pgClassData->oid = oid;

    // table name
    generateGroup(group, 4, 68, tuple);
    string relname(group.begin(), group.end());
    pgClassData->relname = relname;
    // relnamespace
    generateGroup(group, 68, 72, tuple);
    uint32_t relnamespace = convertBigToLittleEndian32(group, 0);
    pgClassData->relnamespace = relnamespace;
    // reltype
    generateGroup(group, 72, 76, tuple);
    uint32_t reltype = convertBigToLittleEndian32(group, 0);
    pgClassData->reltype = reltype;
    // reloftype
    generateGroup(group, 76, 80, tuple);
    uint32_t reloftype = convertBigToLittleEndian32(group, 0);
    pgClassData->reloftype = reloftype;
    // relowner
    generateGroup(group, 80, 84, tuple);
    uint32_t relowner = convertBigToLittleEndian32(group, 0);
    pgClassData->relowner = relowner;
    // relam
    generateGroup(group, 84, 88, tuple);
    uint32_t relam = convertBigToLittleEndian32(group, 0);
    pgClassData->relam = relam;
    // relfilenode

    generateGroup(group, 88, 92, tuple);
    uint32_t relfilenode = convertBigToLittleEndian32(group, 0);
    pgClassData->relfilenode = relfilenode;

    if (tableNodeId == pgClassData->relfilenode) {
        tableObjectID = oid;
        tableRelName = relname;
        printf("find table oid in pg_class as %u \n", tableObjectID);
        if (tableObjectID == 0) {
            throw runtime_error("not match table oid in pg_class");
        }
    }

    delete pgClassData;
    // 其他的暂时不解析了，这里只用到了table name和table oid。先暂停，研究一下pg_attribute表的结构再把这两项放到自定义的表结构体里
//        字段名	数据类型	描述
//        oid	oid	表的对象 ID（Object Identifier），唯一标识该表。
//        relname	name	表的名字。
//        relnamespace	oid	表所属的命名空间（schema），引用 pg_namespace.oid。
//        reltype	oid	表的复合类型（pg_type 中的 oid）。
//        reloftype	oid	如果该表是某类型的表，则表示该类型的 pg_type.oid，否则为 0。
//        relowner	oid	表的所有者，引用 pg_authid.oid。
//        relam	oid	如果是索引，表示其访问方法（Access Method），引用 pg_am.oid。
//        relfilenode	oid	表的物理存储文件节点号，通常对应物理文件名。如果是 0，表示继承其他关系的存储方式。
//        reltablespace	oid	表所在的表空间，引用 pg_tablespace.oid。
//        relpages	int4	表的大小，单位是页面（blocks）。
//        reltuples	float4	表的元组估计数量。
//        relallvisible	int4	表的所有可见页面的数量（对可见性映射位图有帮助）。
//        reltoastrelid	oid	表的 TOAST 表的 OID，如果没有 TOAST 表则为 0。
//        relhasindex	bool	是否有索引。
//        relisshared	bool	是否为共享表（系统表中某些表在所有数据库中共享）。
//        relpersistence	char	持久性标志：p（永久），u（临时），u（未登录表）。
//        relkind	char	表的类型：r（普通表），i（索引），S（序列），v（视图），m（物化视图），c（复合类型）。
//        relnatts	int2	表的列数。
//        relchecks	int2	CHECK 约束的数量。
//        relhasrules	bool	是否有规则。
//        relhastriggers	bool	是否有触发器。
//        relhassubclass	bool	是否有子类（继承的表）。
//        relrowsecurity	bool	是否启用了行级安全策略。
//        relforcerowsecurity	bool	是否强制行级安全策略。
//        relispopulated	bool	对于未加载的物化视图，是否已填充数据。
//        relreplident	char	标识列的类型，用于逻辑复制。
//        relispartition	bool	是否为分区表。
//        relfrozenxid	xid	表中所有数据行的冻结事务 ID。
//        relminmxid	xid	表中所有数据行的最小多事务 ID。
};

int resolvePgAttributeTupeData(vector<uint8_t>& tuple) {

    // 目前只解析指定表的oid的数据就好了
//    Form_pg_attribute pgAttrData = new FormData_pg_attribute;
    vector<Form_pg_attribute> PgAttrVec;
    vector<uint8_t > group;


    generateGroup(group, 0, 4, tuple);
    uint32_t oid = convertBigToLittleEndian32(group, 0);
//    cout << oid << endl;
//    printf("\n begin resolve oid: %d \n", oid);
//    for (int i = 0; i < tuple.size(); ++i) {
//        printf(" %x ", tuple[i]);
//    }
//    printf("\n");

    // 这里没找到32771这个表
    if (tableObjectID == oid) {
        printf("found %u \n", tableObjectID);
        if (tableObjectID == 0) {
            throw runtime_error("not match table oid in pg_attribute");
        }
//        printf("\n begin resolve oid: %d \n", oid);
//        for (int i = 0; i < tuple.size(); ++i) {
//            printf(" %x ", tuple[i]);
//        }
//        printf("\n");
        tableStructData tableStruct;
        tableStruct.oid = oid;
        tableStruct.tableName = tableRelName;
        tableStruct.filenode = tableNodeId;
//        pgAttrData->attrelid = oid;
        // attname
        generateGroup(group, 4, 68, tuple);
        string attname(group.begin(), group.end());

        tableStruct.columnName = attname;
//        pgAttrData->attname = attname;
        // atttypid
        generateGroup(group, 68, 72, tuple);
        uint32_t atttypid = convertBigToLittleEndian32(group, 0);
        string typeName = getColumnTypeName(atttypid);

        tableStruct.columnTypeName = typeName;

        tableStruct.columnTypeId = atttypid;
//        pgAttrData->atttypid = atttypid;
        // attlen
        generateGroup(group, 72, 74, tuple);
        uint16_t attlen = convertBigToLittleEndian16(group, 0);
        tableStruct.columnLength = attlen;
//        pgAttrData->attlen = attlen;
        // attnum
        generateGroup(group, 74, 76, tuple);
        uint16_t attnum = convertBigToLittleEndian16(group, 0);
        tableStruct.columnNum = attnum;
//        pgAttrData->attnum = attnum;
        // attcacheoff
//        generateGroup(group, 76, 80, tuple);
//        uint32_t attcacheoff = convertBigToLittleEndian32(group, 0);
//
//        pgAttrData->attcacheoff = attcacheoff;
        // atttypmod
        generateGroup(group, 80, 84, tuple);
        uint32_t atttypmod = convertBigToLittleEndian32(group, 0);
        tableStruct.columnTypeMod = atttypmod;
//        pgAttrData->atttypmod = atttypmod;
        // attndims
//        generateGroup(group, 84, 86, tuple);
//        uint32_t attndims = convertBigToLittleEndian32(group, 0);
//
//        pgAttrData->attndims = attndims;
        // attbyval
//        generateGroup(group, 86, 87, tuple);
//
//        pgAttrData->attbyval = group[0];
        // attalign
//        generateGroup(group, 87, 88, tuple);
//        pgAttrData->attalign = static_cast<char>(group[0]);
        // attstorage
        generateGroup(group, 88, 89, tuple);
        tableStruct.columnStorage = static_cast<char>(group[0]);
//        pgAttrData->attstorage = static_cast<char>(group[0]);
        // attcompression
        generateGroup(group, 89, 90, tuple);
        tableStruct.columnCompression = static_cast<char>(group[0]);
//        pgAttrData->attcompression = static_cast<char>(group[0]);
        // attnotnull
        generateGroup(group, 90, 91, tuple);
        tableStruct.columnNotNull = static_cast<char>(group[0]);
//        pgAttrData->attnotnull = static_cast<char>(group[0]);
        // atthasdef
        generateGroup(group, 91, 92, tuple);
        tableStruct.columnHasDefault = static_cast<char>(group[0]);
//        pgAttrData->atthasdef = static_cast<char>(group[0]);
        // atthasmissing
//        generateGroup(group, 92, 93, tuple);
//        pgAttrData->atthasmissing = static_cast<char>(group[0]);
        // attidentity
//        generateGroup(group, 93, 94, tuple);
//        pgAttrData->attidentity = static_cast<char>(group[0]);
        // attgenerated
//        generateGroup(group, 94, 95, tuple);
//        pgAttrData->attgenerated = static_cast<char>(group[0]);
        // attisdropped
        generateGroup(group, 95, 96, tuple);
        tableStruct.columnIsdropped = static_cast<char>(group[0]);
//        pgAttrData->attisdropped = static_cast<char>(group[0]);
        // attislocal
        generateGroup(group, 96, 97, tuple);
        tableStruct.columnIslocal = static_cast<char>(group[0]);
//        pgAttrData->attislocal = static_cast<char>(group[0]);
        // attinhcount
        generateGroup(group, 97, 99, tuple);
        uint16_t attinhcount = convertBigToLittleEndian16(group, 0);
        tableStruct.columnInhcount = attinhcount;
//        pgAttrData->attinhcount = attinhcount;
        // attcollation
        generateGroup(group, 99, 103, tuple);
        uint32_t attcollation = convertBigToLittleEndian32(group, 0);
        tableStruct.columnCollation = attcollation;
//        tableStructVec.clear();
        tableStructVec.push_back(tableStruct);
//        pgAttrData->attcollation = attcollation;
//        tableStructVec.push_back(pgAttrData);
    }
}

/*
 * 解析pg_class/pg_attribute的header数据
 */
int resolvePgSysTableHeapData(vector<vector<uint8_t> >& tuple, HeapPageHeader HeapHeader, const string& tableMode) {
//    printf(" %s\n", tableMode.c_str());
    for (int i = 0; i < tuple.size(); ++i) {
//        for (int j = 0; j < tuple[i].size(); ++j) {
//            printf(" %x ", tuple[i][j]);
//        }
//        printf("\n");
        if (tuple[i].size() < 2) {
            return 0;
        }
        // 不管pd_linp有没有数据，直接解析
        vector<uint8_t > group;
        // xmin
        generateGroup(group, 0, 4, tuple[i]);
        uint32_t tXMIN= convertBigToLittleEndian32(group, 0);

        // xmax
        generateGroup(group, 4, 8, tuple[i]);
        uint32_t tXMAX = convertBigToLittleEndian32(group, 0);
        // cid
        generateGroup(group, 8, 12, tuple[i]);
        uint32_t tCID = convertBigToLittleEndian32(group, 0);
        // ctid -> blockID
        generateGroup(group, 12, 16, tuple[i]);
        uint32_t tCTID_blockID = convertBigToLittleEndian32(group, 0);
        // ctid -> offsetNumber
        generateGroup(group, 16, 18, tuple[i]);
        uint32_t tCTID_offsetNumer = convertBigToLittleEndian32(group, 0);
        // t_infomask2
        generateGroup(group, 18, 20, tuple[i]);
        uint32_t tInfo2 = convertBigToLittleEndian32(group, 0);
        // t_infomask
        generateGroup(group, 20, 22, tuple[i]);
        uint32_t tInfo = convertBigToLittleEndian32(group, 0);
        // t_hoff
        generateGroup(group, 22, 23, tuple[i]);
//        uint32_t tHoff = convertBigToLittleEndian32(group, 0);
        // t_bits
        // t_bits的长度等于t_hoff
        // t_hoff转换十进制后，对应到t_bits的位数。比如转换为32，则对应是4个字节，由于t_hoff占一个字节，后面4个字节会对不齐，所以要加1个字节
        uint32_t tBits;
        if (group[0] == 32) {
            generateGroup(group, 23, 28, tuple[i]);
            tBits = convertBigToLittleEndian32(group, 0);
        }
        // t_oid
        generateGroup(group, 28, 32, tuple[i]);
        uint32_t tOID = convertBigToLittleEndian32(group, 0);
        // t_data
        generateGroup(group, 32, HeapHeader->pd_linp[i].lp_len, tuple[i]);
        if (tableMode == "pg_class") {
            // group就是真正的tuple内容，对应到pg_class表中的数据内容
            resolvePgClassTupeData(group);
        } else if (tableMode == "pg_attribute") {
            resolvePgAttributeTupeData(group);
        } else if (group.size() == 0) {
            throw runtime_error("no match data in pg_class or pg_attribute. resolvePgSysTableHeapData in table.cpp");
        }




    }
};

/*
 * @param
 * @HeapHeader     heap page header contain pd_linp arry for tuple
 * @logical_page   8192 byte of page. contain all data for header and pd_linp and tuple
 * throught the pd_linp arry to found table data position.
 * HeapHeader->pd_linp->lp_off contain the offset for tuple of pd_linp as page
 */
int parserHeapTupleData(HeapPageHeader HeapHeader, vector<uint8_t>& logical_page, const string& mode, const string& tableMode) {
    vector<vector<uint8_t> > allTuples;

    for (int i = 0; i < HeapHeader->pd_linp.size(); ++i) {
        // 创建一个新的 vector<uint8_t> 来存储当前元组的数据
        vector<uint8_t> tupleData;
        tupleData.resize(HeapHeader->pd_linp[i].lp_len + 1);  // 为当前元组分配足够的空间

//        if (HeapHeader->pd_linp[i].lp_off > 8192 || HeapHeader->pd_linp[i].lp_off == 0) {
//            continue;
//        }
        // 将数据拷贝到新的 vector 中
        memcpy(tupleData.data(), logical_page.data() + HeapHeader->pd_linp[i].lp_off, HeapHeader->pd_linp[i].lp_len);
        // 将当前元组数据添加到结果容器中
        allTuples.push_back(tupleData);
        tupleData.clear();
    }
    if (mode == "parserDataRecover") {
        printf("begin parser table data\n");
        resolveTupleData(allTuples, HeapHeader);
    } else if (mode == "parserTableConstruct") {
        // 解析表结构
        resolvePgSysTableHeapData(allTuples, HeapHeader, tableMode);
    }

}

uint16_t resolveHeaderStruct(vector<uint8_t>& logical_page, const string& mode, const string& tableMode, int pageNumber) {
    HeapPageHeader HeapHeader = new HeapPageHeaderData;
    // initialize pdlinp vector

    HeapHeader->pd_lsn.xlogid = convertBigToLittleEndian32(logical_page, 0);
    HeapHeader->pd_lsn.xrecoff = convertBigToLittleEndian32(logical_page, 4);
    HeapHeader->pd_checksum = convertBigToLittleEndian16(logical_page, 8);
    HeapHeader->pd_flags = convertBigToLittleEndian16(logical_page, 10);
    HeapHeader->pd_lower = convertBigToLittleEndian16(logical_page, 12);
    HeapHeader->pd_upper = convertBigToLittleEndian16(logical_page, 14);
    HeapHeader->pd_special = convertBigToLittleEndian16(logical_page, 16);
    HeapHeader->pd_pagesize_version = convertBigToLittleEndian16(logical_page, 18);
    HeapHeader->pd_prune_xid = convertBigToLittleEndian32(logical_page, 20);
    HeapHeader->page_number = pageNumber;

#ifdef OPENGAUSS
    HeapHeader->pd_xid_base = convertBigToLittleEndian16(stoull(accumulate(pageHeader.begin() + 24, pageHeader.end() + 32, std::string()), nullptr, 16));
    HeapHeader->pd_multi_base = convertBigToLittleEndian16(stoull(accumulate(pageHeader.begin() + 32, pageHeader.end() + 40, std::string()), nullptr, 16));
#endif

#ifdef __APPLE__
    // 将数据向前移动24字节
//    std::memmove(logical_page.data(), logical_page.data() + HeapPageHeaderSize, logical_page.size() - HeapPageHeaderSize);
    // 调整大小
//    logical_page.resize(logical_page.size() - HeapPageHeaderSize);
    HeapHeader->pd_linp.reserve(HeapHeader->pd_lower - HeapPageHeaderSize);
    HeapHeader->pd_linp.clear();
    // 跳过头部24字节
    for (size_t i = HeapPageHeaderSize; i < logical_page.size(); i += 4) {
        if (i == HeapHeader->pd_lower) {
            break;
        }
        vector<uint8_t> group;
        group.reserve(HeapHeader->pd_lower - HeapPageHeaderSize);
        group.clear();
        for (size_t j = i; j < i + 4 && j < logical_page.size(); ++j) {
            group.emplace_back(logical_page[j]);
        }
        // 转换二进制
        uint32_t pdlinpArry = convertBigToLittleEndian32(group, 0);
        ItemIdData pdlinp;
        pdlinp.lp_len = pdlinpArry >> 17;
        pdlinp.lp_flags = (pdlinpArry >> 1) & 1;
        pdlinp.lp_off = (pdlinpArry & 0x7FFF);

        HeapHeader->pd_linp.push_back(pdlinp);
    }

    // 解析具体的数据内容
    parserHeapTupleData(HeapHeader, logical_page, mode, tableMode);


    logical_page.clear();
    delete HeapHeader;
#elif defined(__linux__)
    vector<uint8_t> logicPage1PDLinp(logical_page.begin(), logical_page.begin() + HeapHeader->pd_lower - HeapPageHeaderSize);
    int k = 0;
    for (size_t i = 0; i < logicPage1PDLinp.size(); i += 4) {
        std::vector<uint8_t> group;
        for (size_t j = i; j < i + 4 && j < logicPage1PDLinp.size(); ++j) {
            group.push_back(logicPage1PDLinp[j]);
        }
        // 转换二进制
        uint32_t pdlinpArry = convertBigToLittleEndian32(group, 0);
        bitset<32> pdLinp(pdlinpArry);
        HeapHeader->pd_linp[k].lp_len = pdlinpArry >> 17;
        HeapHeader->pd_linp[k].lp_flags = (pdlinpArry >> 1) & 1;
        HeapHeader->pd_linp[k].lp_off = (pdlinpArry & 0x7FFF);
        k++;
    }

    logicPage1PDLinp.clear();
    delete HeapHeader;
#endif
    return 0;
}

int findTableStruct(string& pgClassNodePath, const string& tableMode) {
    // 打开pg_class文件，找到表名
#ifdef __APPLE__
    int fd = tableOnDiskOpen(pgClassNodePath, 10);
    off_t fileSize = lseek(fd, 0, SEEK_END);
    int num;
    if (fileSize == 8192) {
        num = 1; // 有多少页
    } else {
        num = fileSize / _PAGESIZE;
    }

    for (int i = 0; i < num; ++i) {
        void* recoverPageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * i);
        if (recoverPageHeap == MAP_FAILED) {
            perror("PageHeaderDATA mmap failed.");
            close(fd);
            return 1;
        }
        vector<uint8_t > logical_page1(_PAGESIZE + 1);
        vector<uint8_t > logical_page2(8192 + 1);
        logical_page1.reserve(_PAGESIZE + 1);
        logical_page2.reserve(_PAGESIZE / 2);
        memcpy(logical_page1.data(), recoverPageHeap, _PAGESIZE);
        memcpy(logical_page2.data(), logical_page1.data() + 8192, 8192);
        logical_page1.resize(_PAGESIZE / 2);
        munmap(recoverPageHeap, _PAGESIZE);
        resolveHeaderStruct(logical_page1, "parserTableConstruct", tableMode, i);
        if (fileSize != 8192) {
            resolveHeaderStruct(logical_page2, "parserTableConstruct", tableMode, i);
        }
        logical_page1.clear();
        logical_page2.clear();
    }
    tableOnDiskClose(fd, 100);

#elif defined(__linux__)
    off_t fileSize = lseek(fd, 0, SEEK_END);
    int num = fileSize / _PAGESIZE; // 有多少页
    for (int i = 0; i < num; ++i) {
        void* recoverPageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * i);
        if (recoverPageHeap == MAP_FAILED) {
            perror("PageHeaderDATA mmap failed.");
            close(fd);
            return 1;
        }
        vector<uint8_t > logical_page(_PAGESIZE + 1);
        logical_page.reserve(_PAGESIZE + 1);
        memcpy(logical_page.data(), recoverPageHeap, _PAGESIZE);
        munmap(recoverPageHeap, _PAGESIZE);
        resolveHeaderStruct(logical_page);
        logical_page.clear();
    }
#endif
    return 0;

}

/*
 * 传入的oid：/Users/haruhi/ClionProjects/postgresql-17.2/pg_install/data/base/16384/49750
 * 1、获取到表oid的目录
 * 2、根据目录，找到目录下的filenode.map文件
 * 3、filenode.map文件中记录了oid : filenode的信息，oid占4个字节，filenode占4个字节
 * 4、找到pg_class表（oid = 1259 = 04eb）和pg_attribute表（oid = 1249 = 04e1）的filenode物理文件
 * 5、通过传入的表oid，到pg_class文件中找到表名。通过oid，到pg_attribute物理文件中找到表对应的列信息
 * 6、拼接表名+列属性，形成完整的表结构
 */
int handleTableConstructRecover(const string& oid) {
    size_t lastSlashPos = oid.find_last_of('/');
    string oidPath, tableNodeIdWithString;
    if (lastSlashPos != std::string::npos) {
        oidPath = oid.substr(0, lastSlashPos);
        tableNodeId = static_cast<uint32_t>(std::stoul(oid.substr(lastSlashPos + 1, 50)));
        tableNodeIdWithString = oid.substr(lastSlashPos + 1, 50);
    }
    vector<uint8_t > nodeMap;
    off_t fileSize;
    for (const auto& entry : directory_iterator(oidPath)) {
        string file_path = entry.path().filename().string();
        if (file_path.rfind("pg_filenode.map", 0) == 0) {
            file_path = oidPath + "/" + file_path;
            int fd = tableOnDiskOpen(file_path, 10);
            fileSize = lseek(fd, 0, SEEK_END);
            nodeMap.resize(fileSize + 1);
            void* FilenodeMap = mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
            memcpy(nodeMap.data(), FilenodeMap, fileSize);
        }
    }
    // 每8个字节为一组
    vector<vector<uint8_t> > nodeGroup;
    for (size_t i = 0; i < nodeMap.size(); i += 8) {
        vector<uint8_t> group;
        for (size_t j = i; j < i + 8 && j < nodeMap.size(); ++j) {
            group.emplace_back(nodeMap[j]);
        }
        nodeGroup.push_back(group);
    }
    uint32_t pgClassNode, pgAttributeNode;
    for (const auto& group : nodeGroup) {
        // 提取前4字节
        std::vector<uint8_t> mapOid(group.begin(), group.begin() + 4);
        uint32_t oidNum = convertBigToLittleEndian32(mapOid, 0);
        // 提取后4字节
        std::vector<uint8_t> mapNode(group.begin() + 4, group.end());
        uint32_t nodeNum = convertBigToLittleEndian32(mapNode, 0);
        // 通过表的filenode找到pg_class和pg_attribute表的oid
        // 普通表需要通过pg_class表找到oid
        if (oidNum == 1259) {
            pgClassNode = nodeNum;
        } else if (oidNum == 1249) {
            pgAttributeNode = nodeNum;
        }
    }

    // 到pg_class中查找表名
    // 找到pg_class的filenode名
    // 拼接全路径
    // 这里只需要执行一次，如果是大文件的话，有 xxx xxx.1 xxx.2这样，这里判断一下
    if (tableNodeIdWithString.find_last_of('.') == -1) {
        string pgClassNodePath = oidPath + "/" + to_string(pgClassNode);
        findTableStruct(pgClassNodePath, "pg_class");
        string pgAttributeNodePath = oidPath + "/" + to_string(pgAttributeNode);
        findTableStruct(pgAttributeNodePath, "pg_attribute");
        oidPath.clear();
    }



    // 到pg_attribute中找到列

//    string pgAttributeNodePath = oidPath + "/" + to_string(pgAttributeNode);
////    findTableColums(pgAttributeNodePath);
//    oidPath.clear();
}

int handleHeapDataRecover(int fd, size_t addrIndex) {
#ifdef __APPLE__

    // 全表恢复部分
    // 如果有索引的话，通过索引确定到某一页。如果没有的话，就全表扫描
    // 一次读取2页的数据
    // addrIndex: 从0递增，乘以页大小
    off_t fileSize = lseek(fd, 0, SEEK_END);
    int num;
    if (fileSize == 8192) {
        num = 1; // 有多少页
    } else {
        num = fileSize / _PAGESIZE;
    }

    for (int i = 0; i < num; ++i) {
        void* recoverPageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * i);
        if (recoverPageHeap == MAP_FAILED) {
            perror("PageHeaderDATA mmap failed.");
            close(fd);
            return 1;
        }
        vector<uint8_t > logical_page1(_PAGESIZE + 1);
        vector<uint8_t > logical_page2(8192 + 1);
        logical_page1.reserve(_PAGESIZE + 1);
        logical_page2.reserve(_PAGESIZE / 2);
        memcpy(logical_page1.data(), recoverPageHeap, _PAGESIZE);
        memcpy(logical_page2.data(), logical_page1.data() + 8192, 8192);
        logical_page1.resize(_PAGESIZE / 2);
        munmap(recoverPageHeap, _PAGESIZE);
        resolveHeaderStruct(logical_page1, "parserDataRecover", "1", i);
        if (fileSize != 8192) {
            resolveHeaderStruct(logical_page2, "parserDataRecover", "1", i);
        }
        logical_page1.clear();
        logical_page2.clear();
    }
    tableOnDiskClose(fd, 100);

#elif defined(__linux__)
    void* recoverPageHeap = mmap(nullptr, _PAGESIZE, PROT_READ, MAP_PRIVATE, fd, _PAGESIZE * addrIndex);
    if (recoverPageHeap == MAP_FAILED) {
        perror("PageHeaderDATA mmap failed.");
        close(fd);
        return 1;
    }
    vector<uint8_t > logical_page(_PAGESIZE + 1);
    logical_page.reserve(_PAGESIZE + 1);
    memcpy(logical_page.data(), recoverPageHeap, _PAGESIZE);
    munmap(recoverPageHeap, _PAGESIZE);
    resolveHeaderStruct(logical_page);
    logical_page.clear();
#endif
    return 0;
}

int processDataRecover(std::map<std::string, std::map<std::vector<string >, std::vector<HeapPageHeader> > > tableOidHeapHeader, vector<string> tableList, vector<string>& oidGBList) {
    size_t PAGE_SIZE;


    const size_t NUM_THREADS = 4;  // 线程数：4个线程

    std::mutex mtx;  // 用于同步线程之间的输出
    std::condition_variable cv;  // 用于控制线程间的同步
    size_t current_page = 0;  // 当前要读取的页数
    // 这里是读取数据文件的功能

    vector<thread> smallTableHandleThreads;
    for (int i = 0; i < tableOidHeapHeader.size(); ++i) {
        for (const auto& oidListPair : tableOidHeapHeader[tableList[i]]) {
            if (oidListPair.first.size() == 1) {
                // 小于1G的表，也就是只有一个oid数据文件
                for (string oid: oidListPair.first) {
                    printf(" begin process table oid: %s \n", oid.c_str());
                    struct stat info;
                    stat(oid.c_str(), &info);
                    size_t FILE_SIZE = info.st_size;

                    while (true) {
                        size_t start_page, end_page;
                        {
                            size_t total_pages = FILE_SIZE / _PAGESIZE + (FILE_SIZE % _PAGESIZE > 0 ? 1 : 0);
                            std::lock_guard<std::mutex> lock(mtx);
                            if (current_page >= total_pages) {
                                break;  // 如果已读取完所有页，退出循环
                            }

                            // 每次开启4个线程读取连续的4页
                            start_page = current_page;
                            end_page = std::min(start_page + NUM_THREADS, FILE_SIZE / _PAGESIZE);
                            current_page = end_page;  // 更新当前页
                        }

                        int fd = tableOnDiskOpen(oid, 10);
                        lseek(fd, 0, SEEK_SET); // 重置文件偏移量

                        // 表结构
                        handleTableConstructRecover(oid);
                        // 表数据
                        handleHeapDataRecover(fd, 0);
                        // 模拟线程处理页数据
//                        vector<thread> tableHandleRecoverThreads;
//                        for (size_t i = start_page; i < end_page; ++i) {
//                            tableHandleRecoverThreads.emplace_back([&fd, i]() {
//                                handleHeapDataRecover(fd, i);
//                            });
//                        }
//
//                        // 等待所有线程完成
//                        for (auto& t : tableHandleRecoverThreads) {
//                            t.join();
//                        }
                        // 等待所有线程完成
//                        for (auto& t : tableHandleThreads) {
//                            t.join();
//                        }

                        tableOnDiskClose(fd, 100);
                        break;
                    }
                }
            } else {
                // 大于1G的表，多个oid数据文件

                for (string oid: oidListPair.first) {
//                    printf(" begin process table oid: %s \n", oid.c_str());
                    struct stat info;
                    stat(oid.c_str(), &info);
                    size_t FILE_SIZE = info.st_size;
                    while (true) {
                        size_t start_page, end_page;
                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            if (current_page >= FILE_SIZE / _PAGESIZE) {
                                break;  // 如果已读取完所有页，退出循环
                            }

                            // 每次开启4个线程读取连续的4页
                            start_page = current_page;
                            end_page = std::min(start_page + NUM_THREADS, FILE_SIZE / _PAGESIZE);
                            current_page = end_page;  // 更新当前页
                        }
                        int fd = tableOnDiskOpen(oid, 10);
                        lseek(fd, 0, SEEK_SET); // 重置文件偏移量
                        // 表结构
                        handleTableConstructRecover(oid);
                        // 表数据
                        handleHeapDataRecover(fd, 0);
                        // 模拟线程处理页数据
//                        vector<thread> tableHandleRecoverThreads;
//                        for (size_t i = start_page; i < end_page; ++i) {
//                            tableHandleRecoverThreads.emplace_back([&fd, i]() {
//                                handleHeapDataRecover(fd, i);
//                            });
//                        }
//
//                        // 等待所有线程完成
//                        for (auto& t : tableHandleRecoverThreads) {
//                            t.join();
//                        }
                        // 等待所有线程完成
//                        for (auto& t : tableHandleThreads) {
//                            t.join();
//                        }

                        tableOnDiskClose(fd, 100);
                        break;
                    }


                }
            }
        }
    }
    return 0;
}

int processRecover() {
    auto it = tableStructMap.find(tableRelName);
    const size_t NUM_THREADS = 4;  // 线程数：4个线程

    std::mutex mtx;  // 用于同步线程之间的输出
    std::condition_variable cv;  // 用于控制线程间的同步
    size_t current_page = 0;  // 当前要读取的页数
    // 这里是读取数据文件的功能

    if (it != tableStructMap.end()) {
        std::vector<tableConstructData>& tableConstructVec = it->second;

        if (stoi(tableConstructVec[0].tableSize) < 1073741824) {
            printf(" begin process table oid: %s \n", tableConstructVec[0].filenodePath.c_str());
            struct stat info;
            stat(tableConstructVec[0].filenodePath.c_str(), &info);
            size_t FILE_SIZE = info.st_size;

            while (true) {
                size_t start_page, end_page;
                {
                    size_t total_pages = FILE_SIZE / _PAGESIZE + (FILE_SIZE % _PAGESIZE > 0 ? 1 : 0);
                    std::lock_guard<std::mutex> lock(mtx);
                    if (current_page >= total_pages) {
                        break;  // 如果已读取完所有页，退出循环
                    }

                    // 每次开启4个线程读取连续的4页
                    start_page = current_page;
                    end_page = std::min(start_page + NUM_THREADS, FILE_SIZE / _PAGESIZE);
                    current_page = end_page;  // 更新当前页
                }

                int fd = tableOnDiskOpen(tableConstructVec[0].filenodePath, 10);
                lseek(fd, 0, SEEK_SET); // 重置文件偏移量

                // 表数据

                handleHeapDataRecover(fd, 0);
                // 模拟线程处理页数据
//                        vector<thread> tableHandleRecoverThreads;
//                        for (size_t i = start_page; i < end_page; ++i) {
//                            tableHandleRecoverThreads.emplace_back([&fd, i]() {
//                                handleHeapDataRecover(fd, i);
//                            });
//                        }
//
//                        // 等待所有线程完成
//                        for (auto& t : tableHandleRecoverThreads) {
//                            t.join();
//                        }
                // 等待所有线程完成
//                        for (auto& t : tableHandleThreads) {
//                            t.join();
//                        }

                tableOnDiskClose(fd, 100);
                break;
            }
        }

    } else {
        throw runtime_error("table name not found in tableStructMap");
    }
}


int InitTable(vector<string>& tabList, const string& dbName, const string& tableName) {
    vector<string> tabOidList;
    vector<string > oidGBList;
    string DBPath, DBDIRPath;

//    if (tableName == "fromFIleOID") {
//        // 直接传入的oid文件
//        // 传入的有可能是当前目录的，直接是一个oid号，也有可能是一个全路径
//        printf("not support file oid mode.");
//        return 1;
//    } else {
    InitEventFuncMap();
    getPGAttributeWithTable(tableName);
    // 通过表名找到file node,加入到tableStructMap中
//    generateCreateTableStatement();
    processRecover();
//    getTableNameFromDatabase(dbName, tabList, tableName);
//    // tabOidList: base/xxx/xxxx
//    getTableOID(tabList, tabOidList, dbName, "haruhi", "123456");
//    // oidGBList: vector for oid of full path
//    checkTableOIDOnDisk(tabOidList, DBPath, DBDIRPath, tabList, oidGBList);
//    // 初始化类型函数
////    InitEventFuncMap();
//    // handle for table
////        processHeapHeaderArry(tableOidHeapHeader, tabList, oidGBList);
//
//    // 单表模式 可能有多个数据文件 recover signal table mode
//    processDataRecover(tableOidHeapHeader, tabList, oidGBList);
//    generateCreateTableStatement();
//    }
    return 0;
}



char byteToHex(char& byte) {
    std::ostringstream hexStream;
    hexStream << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)byte;
//    std::string hexString = hexStream.str();

//    std::cout << "Hexadecimal representation: " << hexString << std::endl;
//    printf(" hexStream[0]: %x \n", hexStream.str()[0]);
//    char res = (hexStream.str()[0]) << 4 | (hexStream.str()[1]);
    return static_cast<char>(std::stoi(hexStream.str(), nullptr, 16));
//    return res;
}

string vectorCharConvertToString(vector<char> pageHeader, int beginPos, int endPos) {
    string HeaderStr(pageHeader.begin() + beginPos, pageHeader.begin() + endPos);
    return HeaderStr;
}


/*
 * 生成表结构
 */
int generateCreateTableStatement() {
    auto it = tableStructMap.find(tableRelName);

    if (it != tableStructMap.end()) {
        std::vector<tableConstructData>& tableConstructVec = it->second;
        string tableCreateStmt = "CREATE TABLE " + tableRelName + " ( \n";

        for (int i = 0; i < tableConstructVec.size(); ++i) {
            if (tableConstructVec[i].columnNum >= "65530" || tableConstructVec[i].columnNum < "0" ) {
                continue;
            }
            printf(" columnTypeName: %s ", tableConstructVec[i].columnTypeName.c_str());
            tableCreateStmt.append(tableConstructVec[i].columnName).append(" ").append(tableConstructVec[i].columnTypeName)
            .append(" (").append(tableConstructVec[i].columnLength == "65535" ? tableConstructVec[i].columnTypeMod : tableConstructVec[i].columnLength)
            .append(") ").append(tableConstructVec[i].columnNotNull == "f" ? "NULL" : "NOT NULL").append("\n");
//            tableCreateStmt += tableConstructVec[i].columnName + " "
//                    + tableConstructVec[i].columnTypeName + "("
//                    + tableConstructVec[i].columnLength == "65535" ? tableConstructVec[i].columnTypeMod : tableConstructVec[i].columnLength + ") "
//                    + (tableConstructVec[i].columnNotNull == "f" ? "NULL" : "NOT NULL") + ", \n";
//        if (i != tableStructVec.size() - 1) {
//            tableCreateStmt += ", \n";
//        }
//                + (tableStructVec[i].columnHasDefault ? );  先不实现默认值了
        }
        tableCreateStmt += ");";

        cout << tableCreateStmt << endl;
    }
//    string tableCreateStmt = "CREATE TABLE " + tableRelName + " ( \n";
//
//    for (int i = 0; i < tableStructVec.size(); ++i) {
//
//        if (tableStructVec[i].columnNum >= 65530) {
//            continue;
//        }
////        cout << tableStructVec[i].columnName << endl;
//        tableCreateStmt += tableStructVec[i].columnName
//                + " " + tableStructVec[i].columnTypeName + "("
//                + to_string(tableStructVec[i].columnLength == 65535 ? tableStructVec[i].columnTypeMod : tableStructVec[i].columnLength) + ") "
//                + (tableStructVec[i].columnNotNull ? "NULL" : "NOT NULL") + ", \n";
////        if (i != tableStructVec.size() - 1) {
////            tableCreateStmt += ", \n";
////        }
////                + (tableStructVec[i].columnHasDefault ? );  先不实现默认值了
//    }
//    tableCreateStmt += ");";
//
//    cout << tableCreateStmt << endl;
};