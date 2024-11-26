//
// Created by 白杰 on 2024/4/22.
//

#include "util.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fcntl.h>
#include <libconfig.h++>
#include <cstdlib>
#include "config.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include "read.h"
#include "auth.h"
#include "packet.h"
#include "send.h"
#include "binlogTypes.h"
#include <libpq-fe.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include<sys/stat.h>
#include<unistd.h>
#include "walProcess.h"
#include <thread>



using namespace libconfig;
using namespace std;

//class ReplicaterConfig GlobalVar;

struct GlobalVar globalVar;
extern struct mysql_conn_info MySQLConnInfo;

bool Util::OpenCfgFile(char * filename) {

    if (access(filename, F_OK | R_OK) == 0)
    {
        printf("load config file");
        Config cfg;
        cfg.readFile(filename);
        Setting & root = cfg.getRoot();
        if (root.exists("MySQL_Host"))
        {
            root.lookupValue("MySQL_Host", globalVar.MySQL_Host);
        }
        if ( root.exists("MySQL_Port") )
        {
            root.lookupValue("MySQL_Port", globalVar.MySQL_Port);
        }
        if (root.exists("MySQL_DataBaseName")) {
            root.lookupValue("MySQL_DataBaseName", globalVar.MySQL_DataBaseName);
        }
        if ( root.exists("MySQL_Username") )
        {
            root.lookupValue("MySQL_Username", globalVar.MySQL_Username);
        }
        if ( root.exists("MySQL_Password") )
        {
            root.lookupValue("MySQL_Password", globalVar.MySQL_Password);
        }
        if ( root.exists("MySQL_BinlogName") )
        {
            root.lookupValue("MySQL_BinlogName", globalVar.MySQL_BinlogName);
        }
        if ( root.exists("MySQL_BinlogPos") )
        {
            root.lookupValue("MySQL_BinlogPos", globalVar.MySQL_BinlogPos);
        }
        if ( root.exists("uuid") )
        {
            root.lookupValue("uuid", globalVar.uuid);
        }
        if (root.exists("MySQL_ID")) {
            root.lookupValue("MySQL_ID", globalVar.MySQL_ID);
        }
        if (root.exists("MySQL_Log_Pos")) {
            root.lookupValue("MySQL_Log_Pos", globalVar.Log_Pos);
        }


        if (root.exists("mysql_dsn_param") ) {
            const Setting &mysqlVar = root["mysql_dsn_param"];
            mysqlVar.lookupValue("AllowAllFiles", globalVar.MySQL_AllowAllFiles);
            mysqlVar.lookupValue("AllowCleartextPasswords", globalVar.MySQL_AllowCleartextPasswords);
            mysqlVar.lookupValue("AllowFallbackToPlaintext", globalVar.MySQL_AllowFallbackToPlaintext);
            mysqlVar.lookupValue("AllowNativePasswords", globalVar.MySQL_AllowNativePasswords);
            mysqlVar.lookupValue("AllowOldPasswords", globalVar.MySQL_AllowOldPasswords);
            mysqlVar.lookupValue("CheckConnLiveness", globalVar.MySQL_CheckConnLiveness);
            mysqlVar.lookupValue("ClientFoundRows", globalVar.MySQL_ClientFoundRows);
            mysqlVar.lookupValue("ColumnsWithAlias", globalVar.MySQL_ColumnsWithAlias);
            mysqlVar.lookupValue("InterpolateParams", globalVar.MySQL_InterpolateParams);
            mysqlVar.lookupValue("MultiStatements", globalVar.MySQL_MultiStatements);
            mysqlVar.lookupValue("ParseTime", globalVar.MySQL_ParseTime);
            mysqlVar.lookupValue("RejectReadOnly", globalVar.MySQL_RejectReadOnly);
            mysqlVar.lookupValue("TLS", globalVar.MySQL_TLS);
            mysqlVar.lookupValue("Collation", globalVar.MySQL_Collation);

        }
        return true;
    }
    return false;
};

//extern class Packet *pck;
int Util::Init() {
    char current_path[FILENAME_MAX];

    if (getcwd(current_path, FILENAME_MAX) != NULL ) {
        printf("%s", current_path);
    }

    char * filename = "../config.cfg";
    if( !OpenCfgFile(filename) ) {
        printf("please check 'config.cfg' file is exists.");
        return 0;
    }
    class Read *netPacket = nullptr;
    // 完成读包操作，读取到全局结构体里
    netPacket->Init();
    // client认证操作
    class Auth *auth = nullptr;
    auth->Init();
    // command阶段
    class Send send;
    send.Iint();
    // 初始化binlog func map
    // 对应MYSQL_TYPE_LONG这些类型
    InitMySQLTypeFuncMaps();

//    send.writeCommandPacketStr("COM_REGISTER_SLAVE", "");

//    send.writeCommandPacketStr("COM_QUERY", query);
//    send.writeCommandPacketStr("COM_BINLOG_DUMP", "");
    const char query[] = "SET @master_binlog_checksum=@@global.binlog_checksum";
//    const char query[] = "select @@version_comment";
    int queryResault = send.writeCommandPacketStr("COM_QUERY", query);
    if (queryResault == 0) {
        send.writeCommandPacketStr("COM_BINLOG_DUMP", "");
    }
    close(MySQLConnInfo.fd);

//    printf("\nauthData\n");
//    for (int i = 0; i < 120; ++i) {
//        printf(" %#x ", MySQLClientSendInfo.PacketBody[i]);
//    }
//    int ret = auth->writePacket();

    return 0;

};

Util::Util()
{

}

int Util::createPostgreSQL() {
    printf("\n createPostgreSQL \n");
    // 初始化连接字符串
    try {
        // 连接数据库字符串，格式为 "host=hostname user=username dbname=database password=password"
//        const char *conninfo = "hostaddr=123.57.78.31 port=5432 dbname=test user=sync password=123456";

        // 通过连接字符串设置连接参数
        const char *conninfo = "host=123.57.78.31 dbname=test user=sync password=123456";

        // 用于存储连接和查询结果的指针
        PGconn *conn;
        PGresult *res;

        // 建立连接
        conn = PQconnectdb(conninfo);

        // 检查连接状态
        if (PQstatus(conn) != CONNECTION_OK) {
            std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl;
            PQfinish(conn); // 关闭连接
            return 1;
        }

        // 执行查询
        res = PQexec(conn, "SELECT * FROM full_type_table");

        // 检查查询执行是否成功
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            std::cerr << "Query execution failed: " << PQerrorMessage(conn) << std::endl;
            PQclear(res); // 释放结果对象
            PQfinish(conn); // 关闭连接
            return 1;
        }

        // 处理查询结果
        int rows = PQntuples(res); // 获取结果行数
        for (int i = 0; i < rows; ++i) {
            std::cout << "Row " << i << ": ";
            for (int j = 0; j < PQnfields(res); ++j) {
                std::cout << PQgetvalue(res, i, j) << "\t"; // 获取单元格值
            }
            std::cout << std::endl;
        }

        // 释放结果对象
        PQclear(res);

        // 关闭连接
        PQfinish(conn);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

bool Util::WriteResumeFile() {
//    ifstream ss();

    return false;
}

bool Util::openFile(const string& filename) {
//    if (access(filename, F_OK | R_OK) == 0) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }
    printf("filename: %s ", filename.c_str());

    // 创建一个vector来存储文件内容
    std::vector<uint8_t> walData;

    // 读取文件内容到vector中
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    walData.resize(fileSize);

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(walData.data()), fileSize);

    startWalParser(walData);


    // 创建一个vector来存储拆分后的8KB块
//    std::vector<std::vector<uint8_t> > sectors;
//    const std::size_t chunkSize = 8192;  // 8KB大小

    // 读取文件内容并拆分为8KB块
//    while (file) {
//        std::vector<uint8_t> chunk(chunkSize);
//        file.read(reinterpret_cast<char*>(chunk.data()), chunkSize);
//        std::streamsize bytesRead = file.gcount();
//
//        if (bytesRead > 0) {
//            chunk.resize(bytesRead);  // 调整块的大小以匹配实际读取的字节数
//            sectors.push_back(chunk);
//        }
//    }

    // 读取文件内容到 vector 中
    file.close();


//        vector<int> walThread;
    // 打印读取的内容
//    std::cout << "Read " << walData.size() << " bytes from the file:" << std::endl;

    // 线程的数量

//    vector<std::thread> walReadThreads; // 存储线程的vector
//    auto walFun = [](int i) mutable -> thread {
//        printf("\naaa\n");
//    };
//    walReadThreads.reserve(sectors.size());
//    for (int i = 0; i < sectors.size(); ++i) {
////        walReadThreads.emplace_back([]()
////                                             {
////                                                 std::cout<<"thread function\n";
////                                             }); // 启动线程并传递线程编号
//    //            walReadThreads.push_back(thread(startWalProcess(i))); // 启动线程并传递线程编号
//    walReadThreads.emplace_back(startWalProcess, i, sectors[i]);
//
//    }
//    std::for_each(walReadThreads.begin(), walReadThreads.end(), [](std::thread & thr) {
//        thr.join();
//
//    });
        std::cout << std::endl;
//    }

    return false;
}
