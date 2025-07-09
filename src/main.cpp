#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include "common/util.h"
#include <csignal>
#include "process/mySignal.h"
#include <fstream>
#include "access/access.h"
#include <CLI/CLI.hpp>



using namespace std;

//int MysqlReplicater_Admin_Int_module() {
//
//};

//typedef int (*commands)(string&);
//map<string , commands > commandsMap;
//
//int parserHeapAndTuple(string& tableName) {
//    InitAccess();
//}
//
//int parserCharParserHeapAndTuple(string& tableName) {
//    InitAccess();
//}
//
//int help() {
//    // usage
//
//}
//
//int InitCommandsMap() {
//    commandsMap["stream_parser"] = &parserHeapAndTuple;
//    commandsMap["c_parser"] = &parserCharParserHeapAndTuple;
//    commandsMap["help"] = &help;
//
//}

//int createBinlogStartApp(int argc, char* argv[]) {
//    // 创建主应用程序
//    CLI::App app{"A binlog receive program and resolve binlog data."};
//
//    CLI::App* start = app.add_subcommand("start", "start binlog receive program");
//    std::string start_binlog_file_name, start_binlog_expire;
//    start->add_option("-f,--file", start_binlog_file_name, "write data into the file of binlog record")
//            ->expected(1);
//    start->add_option("-e,--expire", start_binlog_expire, "The expire time or the expire record count of binlog record for discard")
//            ->expected(1);
//
//    // 定义"info"子命令
//    CLI::App* info = app.add_subcommand("verbose", "Display command information");
//    bool verbose = false;
//    info->add_flag("-v,--verbose", verbose, "Enable verbose output")
//            ->expected(1);
//
//
//
//    // 解析命令行输入
//    CLI11_PARSE(app, argc, argv);
//
//    // 根据子命令执行相应的操作
//    if (*start) {
//        if (!start_binlog_file_name.empty()) {
//
//        }
//        if (!start_binlog_file_name.empty()) {
////                help();
//        }
//
//    } else if (*info) {
//        std::cout << "Displaying information..." << std::endl;
//        if (verbose) {
////            help();
//        }
//    }
//};

int createParserApp(int argc, char* argv[]) {
    // 创建主应用程序
    CLI::App app{"A parser heap of page for postgresql or opengauss."};

    CLI::App* stream_parser = app.add_subcommand("parser", "parser heap page");
    std::string stream_parser_file_name, stream_parser_db_name, stream_parser_table_name;
    stream_parser->add_option("-f,--file", stream_parser_file_name, "The file of oid")
            ->expected(1);
    stream_parser->add_option("-d,--database", stream_parser_db_name, "The database name")
            ->expected(1);
    stream_parser->add_option("-t,--table", stream_parser_table_name, "The table name")
            ->expected(1);
    stream_parser->add_option("table_construct", stream_parser_table_name, "The table construct dic")
            ->expected(1);

    // 定义"info"子命令
    CLI::App* info = app.add_subcommand("verbose", "Display command information");
    bool verbose = false;
    info->add_flag("-v,--verbose", verbose, "Enable verbose output")
            ->expected(1);



    // 解析命令行输入
    CLI11_PARSE(app, argc, argv);

    // 根据子命令执行相应的操作
    if (*stream_parser) {
        if (!stream_parser_db_name.empty() && !stream_parser_table_name.empty()) {
            // 数据库+表名
            string DBTableName = stream_parser_db_name + "." + stream_parser_table_name;
            InitAccessForDBAndTableName(stream_parser_db_name, stream_parser_table_name);
        } else {
            // 如果param1和param2不是同时出现，则分别输出它们
            if (!stream_parser_db_name.empty()) {
//                help();
            }
            if (!stream_parser_table_name.empty()) {
//                help();
            }
        }
        if (!stream_parser_file_name.empty()) {
            // 先不写单独传入oid的
//            InitAccessForFile(stream_parser_file_name);
        } else {
//            help();
        }
    } else if (*info) {
        std::cout << "Displaying information..." << std::endl;
        if (verbose) {
//            help();
        }
    }
};


int main(int argc, char* argv[]) {
    // detect exception signal
//    registerSignal();
//    Util * util = new Util();
//    Util * util;
//    util->Init();

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <command> <subcommand> [options] <name>" << std::endl;
        return 1;
    }
    // create parser program APP
    createParserApp(argc, argv);
    /*
     * 最终结果样式：
        table_name: xxx
        12,'fwe','数据',123.4243
        94,'fwefifwe','数据',84723.484
            |
            |--93,'fiuejjx','老版本的数据',9474.288
            |
            |--92,'fweefe','老版本的数据',3.4243
            |
            |--91,'fweffffff','老版本的数据',13.453
        72,'fkwiirjkx','数据'，9473.3848
     */

    // create mysql binlog receive program
//    createBinlogStartApp(argc, argv);



//
//    MysqlReplicater_Admin_Int_module();
//    if (ret == 1) {
//        raise(SIGINT);
//    }

    // insert to target db
//    util->createPostgreSQL();


//   util->openFile("wal");


}


