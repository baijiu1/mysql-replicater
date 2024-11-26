//
// Created by 白杰 on 2024/4/22.
//

#include <iostream>
#include "send.h"
#include <string>
#include "config.h"
#include <map>
#include <sys/socket.h>
#include "packet.h"
#include <vector>
#include "parser.h"
#include <malloc/malloc.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <netdb.h>

using namespace std;
extern std::map<std::string, uint8_t> CommandMap;
extern struct mysql_conn_info MySQLConnInfo;
extern struct GlobalVar globalVar;
extern vector<struct ComQueryFieldPayload*> PayloadFields;
class Parser *parser;
vector<vector<uint8_t> > global_segments;


#define BUFFER_SIZE 1024

size_t read_length1(const std::vector<unsigned char>& data) {
    size_t length = 0;
    length |= data[0];
    length |= data[1] << 8;
    length |= data[2] << 16;
    return length;
}

void parse_and_remove_segments(std::vector<uint8_t>& data) {
    size_t pos = 0;
    size_t k = 0;
    global_segments.clear();
    while (pos + 3 < data.size()) {
        // 读取长度
        size_t length = read_length1(data);
//        printf(" data.size(): %lu ", data.size());
//        printf("\n length: %lu ", length);
        pos += 3;
        // 检查是否超出数组范围
        if (pos + length > data.size()) {
            std::cerr << "Error: Invalid length, exceeds array size." << std::endl;
//            break;
        }
        vector<uint8_t> segment(data.begin(), data.begin() + length + 4);
//        for (int i = 0; i < segment.size(); ++i) {
//            printf(" %#x ", segment[i]);
//        }

        global_segments.push_back(segment);
//        printf(" segment.size(): %lu ", segment.size());

        // 移除已处理的数据段
        data.erase(data.begin(), data.begin() + length + 4);
//        pos -= 3; // adjust position after erase
        pos -= 3;


    }
}

int Send::writeCommandPacketStr(const string& command, const char *query) {
    // com_command 组包
    uint8_t SendData[strlen(query) + 1 + 4];
    if (command == "COM_QUIT" || command == "COM_PING") {
        TakerComQuitAndPingData(SendData, command);
    } else if (command == "COM_QUERY" || command == "COM_STMT_PREPARE") {
        if (strlen(query) < 1) {
            printf("Invalid arguments count!");
            return 1;
        }
        TakerComQueryAndPrepareData(SendData, command, query);
    } else if (command == "COM_STMT_CLOSE") {
        if (strlen(query) <= 0) {
            printf("Invalid arguments count!");
            return 1;
        }
        SendData[4] = CommandMap["COM_STMT_CLOSE"];
        // int<4>	statement_id	ID of the prepared statement to close
        SendData[5] = query[0];
        SendData[6] = 0x00;
        SendData[7] = 0x00;
        SendData[8] = 0x00;
    } else if (command == "COM_BINLOG_DUMP") {
        TakerComBinlogDumpData(SendData);
    } else if (command == "COM_BINLOG_DUMP_GTID") {
        TakerComBinlogDumpGTIDData(SendData);
    } else if (command == "COM_REGISTER_SLAVE") {
        // master ID
        const char query[] = "SHOW VARIABLES LIKE 'server_id'";
        writeCommandPacketStr("COM_QUERY", query);
        string masterID = PayloadFields[1]->ResultRows.rowsData.c_str();
        TakerComRegisterSlaveData(SendData, masterID);
    } else {
        printf("Unknown command: %s", command.c_str());
        exit(1);
    }

    // send
    SendDataToServer(SendData);
    if (command == "COM_QUERY" || command == "COM_STMT_PREPARE") {
        char *buffer = (char *) malloc(sizeof(buffer));
        int recvSize = readResultSetHeaderPacket(buffer, 5600);
        char RecvBuffer[recvSize];
        memcpy(RecvBuffer, buffer, recvSize);
        free(buffer);
        parser->parserQuery(RecvBuffer, recvSize);
        return 0;
    } else if (command == "COM_BINLOG_DUMP") {
        std::vector<uint8_t> received_data;
        received_data.clear();
        char buffer[BUFFER_SIZE];
        int recvSize;
        do {
            recvSize = readBinlogResultSetPacket(buffer);
            if (recvSize == 0) {
                printf(" recv 0 bytes ");
                break;
            } else if (recvSize < 0) {
                printf(" recv error ");
                break;
            }
            for(unsigned int i = 0; i < recvSize; ++i) {
                received_data.push_back(buffer[i]);
            }
//            received_data.insert(received_data.end(), buffer, buffer + recvSize);
        } while (recvSize > 0);
        // mem copy
        uint8_t * recvDat = (uint8_t *) malloc(received_data.size());
        memset(recvDat, 0, received_data.size());
//        uint8_t * recvDatUint = (uint8_t *) malloc(received_data.size());
//        char recvData[received_data.size() + 10];
        int recvLen = received_data.size();
        printf(" received_data.size(): %lu ", received_data.size());
        for (int i = 0; i < received_data.size(); ++i) {
            recvDat[i] = received_data[i];
//            printf(" %#x ", recvDat[i]);
        }
        // 这里直接拆分接收到的数据
        // 并行？ 每次取前三个字节，算出总长度，向后取相应的长度后，进行拆分每个binlog events
        int *dataLen = &recvLen;
//        takerBufferHeaderSize(recvDat, dataLen, &eventsData);
        parse_and_remove_segments(received_data);
        // 每次到xid为一组，条件为： 开头为32长度，event type为0x10
//        printf(" recvLen: %d \n", recvLen);
        printf(" global_segments.size(): %lu ", global_segments.size());
        std::cout << "Global segments: " << std::endl;
        std::vector<std::vector<vector<uint8_t> > > tasksList;
        std::vector<std::vector<uint8_t> > tasks;
        std::vector<std::vector<uint8_t> > collected_segments;
        for (size_t i = 0; i < global_segments.size(); ++i) {
            if (global_segments[i].size() >= 10 && global_segments[i][0] == 0x20 && global_segments[i][9] == 0x10) {
                tasks.push_back(global_segments[i]);
                tasksList.push_back(tasks);
//                tasksList.insert(tasksList.end(), global_segments[i].begin(), global_segments[i].end());
//                collected_segments.insert(collected_segments.end(), global_segments.begin(), global_segments.begin() + i);
            }
            tasks.push_back(global_segments[i]);
        }
        // 开tasksList.size()个并发
        printf("\n tasksList.size(): %lu \n", tasksList.size());
        for (int i = 0; i < global_segments.size(); ++i) {
            parser->handleBufferBinlogEvents_test(global_segments[i], dataLen);
        }

        free(recvDat);
        return 0;
    }

    return 0;
}
size_t read_length(uint8_t * data) {
    size_t length = 0;
    length |= data[0];
    length |= data[1] << 8;
    length |= data[2] << 16;
    return length;
}
int Send::takerBufferHeaderSize(uint8_t * data, size_t *size, vector<uint8_t> *eventsData) {
    size_t pos = 0;
    while (pos < *size) {
        // 读取长度
        size_t length = read_length(data);
        printf("\n length: %lu ", length);
//        pos += 3;

        // 检查是否超出数组范围
        if (pos + length > *size) {
            std::cerr << "Error: Invalid length, exceeds array size." << std::endl;
            break;
        }

        for (size_t i = 0; i < length; ++i) {
            eventsData->push_back(data[i]);
        }
        // 把data已经加入到数组到消费掉
        for (int i = 0; i < *size; ++i) {
            data[i] = data[i + length + 4];
        }
        *size -= length + 4;
        // 移动到下一个段
        pos += length;
    }

}



int Send::execQuery(string query) {

    return 0;
}

int Send::SendDataToServer(uint8_t *data) {
    int SendRet = send(MySQLConnInfo.fd, data, data[0] + 4, 0);
    if (SendRet > 0) {
        printf("send ok!");
    } else {
        printf("send filed!exit...");
        exit(1);
    }
}

int Send::readBinlogResultSetPacket(char * RecvBuffer) {
        int RecvRet = recv(MySQLConnInfo.fd, RecvBuffer, BUFFER_SIZE, 0);
        if (RecvRet > 0) {
            printf("receive ok!");
            printf("\n receive size: %d ", RecvRet);
            return RecvRet;
        } else if (RecvRet == 0) {
            printf("receive 0 byte data!");
            return RecvRet;
        } else {
            printf("receive filed!exit...");
            return RecvRet;
        }
}

int Send::readResultSetHeaderPacket(char *RecvBuffer, int length) {
//    while (true) {
        int RecvRet = recv(MySQLConnInfo.fd, RecvBuffer, length, 0);
        if (RecvRet > 0) {
            printf("receive ok!");
            printf("\n receive size: %d ", RecvRet);
//            RecvRet += RecvRet;
            return RecvRet;
        } else if (RecvRet == 0) {
            printf("receive 0 byte data!");
//            continue;
        } else {
            printf("receive filed!exit...");
        }
//    }


}


int Send::TakeBufferHeader(int argCound, ...) {
    va_list args;
    va_start(args, argCound);
    if (argCound == 2) {
        // 只有data + command
        char * arg1 = va_arg(args, char *);
        va_end(args);


    } else {
        // data + sql + command
        char * arg2 = va_arg(args, char *);
        va_end(args);
        char * arg3 = va_arg(args, char *);
        va_end(args);
    }

    return 0;
}


int Send::Iint() {
    MapSet();
    return 0;
}

int Send::TakerComRegisterSlaveData(uint8_t *SendData, string masterID) {
    std::vector<std::string> ip = getLocalIPs();

    int registerLen = 4 + 1 + 4 + strlen(ip.front().c_str()) + strlen(globalVar.MySQL_Username.c_str())
            + strlen(globalVar.MySQL_Password.c_str()) + 2 + 4 + 4;
    SendData[0] = registerLen;
    SendData[1] = registerLen << 8;
    SendData[2] = registerLen << 16;
    SendData[3] = registerLen << 24;
    SendData[4] = CommandMap["COM_REGISTER_SLAVE"];
    SendData[5] = globalVar.MySQL_ID; // slave id
    SendData[6] = globalVar.MySQL_ID << 8; // slave id
    SendData[7] = globalVar.MySQL_ID << 16; // slave id
    SendData[8] = globalVar.MySQL_ID << 24; // slave id
    // 这里应该获取到本机地址
    SendData[9] = strlen(ip.front().c_str()); // Slave host name len
    int pos = 10;
    for (int i = 0; i < strlen(ip.front().c_str()); ++i) {
        if (ip.front()[i] == '\0') {
            break;
        }
        SendData[pos] = ip.front()[i];
        pos ++;
    }
    SendData[pos] = strlen(globalVar.MySQL_Username.c_str());
    pos += 1;
    for (int i = 0; i < strlen(globalVar.MySQL_Username.c_str()); ++i) {
        if (globalVar.MySQL_Username[i] == '\0') {
            break;
        }
        SendData[pos] = globalVar.MySQL_Username[i];
        pos ++;
    }
    SendData[pos] = strlen(globalVar.MySQL_Password.c_str());
    pos += 1;
    for (int i = 0; i < strlen(globalVar.MySQL_Password.c_str()); ++i) {
        if (globalVar.MySQL_Password[i] == '\0') {
            break;
        }
        SendData[pos] = globalVar.MySQL_Password[i];
        pos ++;
    }
    SendData[pos] = (globalVar.MySQL_Port >> 8) & 0xFF;
    pos += 1;
    SendData[pos] = globalVar.MySQL_Port & 0xFF;
    pos += 1;
    for (int i = 0; i < 4; ++i) {
        SendData[pos] = 0x00;
        pos ++;
    }
    SendData[pos] = masterID[0];
    pos += 1;
    SendData[pos] = masterID[0] << 8;
    pos += 1;
    SendData[pos] = masterID[0] << 16;
    pos += 1;
    SendData[pos] = masterID[0] << 24;

    return 0;
}

int Send::TakerComBinlogDumpData(uint8_t *SendData) {
    const char binlogFilename[] = "on.000001";
//        SendToData.push_back();
    SendData[0] = 1 + 4 + 2 + 4 + strlen(binlogFilename);
    SendData[1] = 0x00;
    SendData[2] = 0x00;
    SendData[3] = 0x00; // seq id
    SendData[4] = CommandMap["COM_BINLOG_DUMP"];
    for (int i = 0; i < 4; ++i) {
        SendData[i + 5] = (globalVar.Log_Pos >> (i * 8)) & 0xFF;
    }
    SendData[9] = 0x00; // flags
    SendData[10] = 0x00;
    SendData[11] = globalVar.MySQL_ID; // server-id
    SendData[12] = globalVar.MySQL_ID >> 8; // server-id
    SendData[13] = globalVar.MySQL_ID >> 16; // server-id
    SendData[14] = globalVar.MySQL_ID >> 24; // server-id
    for (int i = 0; i < strlen(binlogFilename); ++i) {
        SendData[i+15] = binlogFilename[i];
    }
    return 0;
}

int Send::TakerComBinlogDumpGTIDData(uint8_t *SendData) {
    /**
    binlog_flags 2
    server_id 4
    binlog-filename-len 4
    binlog name ""
    binlog_pos 8
     gtid_set_encoded_size 4
     gtid_set_encoded_data
    */
    SendData[0] = 1;
    SendData[1] = 0x00;
    SendData[2] = 0x00;
    SendData[3] = 0x00; // seq id
    SendData[4] = CommandMap["COM_BINLOG_DUMP_GTID"];
    return 0;
}

int Send::TakerComQuitAndPingData(uint8_t *SendData, const string& command) {
    SendData[0] = 0x01;
    SendData[1] = 0x00;
    SendData[2] = 0x00;
    SendData[3] = 0x00; // seq id
    if (command == "COM_QUIT") {
        SendData[4] = CommandMap["COM_QUIT"];
    } else {
        SendData[4] = CommandMap["COM_PING"];
    }
    return 0;
}

int Send::TakerComQueryAndPrepareData(uint8_t *SendData, const string& command, const char *query) {
    if (command == "COM_QUERY") {
        SendData[0] = strlen(query) + 1;
        SendData[1] = 0x00;
        SendData[2] = 0x00;
        SendData[3] = 0x00; // seq id
        SendData[4] = CommandMap["COM_QUERY"];
        int DBQueryPos = 5;
        for (int i = 0; i < strlen(query); ++i) {
            if (query[i] == '\0') {
                break;
            }
            SendData[DBQueryPos] = query[i];
            DBQueryPos ++;
        }
    } else {
        SendData[0] = strlen(query) + 1;
        SendData[1] = 0x00;
        SendData[2] = 0x00;
        SendData[3] = 0x00; // seq id
        SendData[4] = CommandMap["COM_STMT_PREPARE"];
        int DBQueryPos = 5;
        for (int i = 0; i < strlen(query); ++i) {
            if (query[i] == '\0') {
                break;
            }
            SendData[DBQueryPos] = query[i];
            DBQueryPos ++;
        }
    }

    return 0;
}

std::vector<std::string> getLocalIPs() {
    std::vector<std::string> ips;

#ifdef _WIN32
    // For Windows
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    ULONG outBufLen = 0;
    DWORD dwRetVal = 0;

    // Make an initial call to get the size needed into the outBufLen variable
    dwRetVal = GetAdaptersAddresses(AF_INET, 0, nullptr, pAddresses, &outBufLen);

    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
        if (pAddresses == nullptr) {
            throw std::runtime_error("Memory allocation failed.");
        }

        dwRetVal = GetAdaptersAddresses(AF_INET, 0, nullptr, pAddresses, &outBufLen);
        if (dwRetVal == NO_ERROR) {
            for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses; pCurrAddresses != nullptr; pCurrAddresses = pCurrAddresses->Next) {
                if (pCurrAddresses->OperStatus == IfOperStatusUp) {
                    IP_ADAPTER_UNICAST_ADDRESS *pUniAddr = pCurrAddresses->FirstUnicastAddress;
                    while (pUniAddr != nullptr) {
                        sockaddr_in *pSockAddr = (sockaddr_in *)pUniAddr->Address.lpSockaddr;
                        char addrBuf[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(pSockAddr->sin_addr), addrBuf, INET_ADDRSTRLEN);
                        ips.push_back(std::string(addrBuf));
                        pUniAddr = pUniAddr->Next;
                    }
                }
            }
        }

        free(pAddresses);
    }
#else
    // For Unix-like systems
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[1025];

    if (getifaddrs(&ifaddr) == -1) {
        throw std::runtime_error("getifaddrs failed.");
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, 1025, nullptr, 0, NI_NUMERICHOST);
            if (s == 0) {
                ips.push_back(host);
            } else {
                throw std::runtime_error("getnameinfo failed.");
            }
        }
    }

    freeifaddrs(ifaddr);
#endif

    return ips;
}
