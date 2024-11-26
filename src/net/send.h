//
// Created by 白杰 on 2024/4/22.
//

#ifndef MYSQL_REPLICATER_SEND_H
#define MYSQL_REPLICATER_SEND_H



#include <string>

using namespace std;

class Send {
public:
    int Iint();
    int writeCommandPacketStr(const string& command, const char *query);
    int execQuery(string query);
    int readResultSetHeaderPacket(char *RecvBuffer, int length);
    int readResultSetHeaderPacketTest(vector<char> RecvBuffer, int length);

    int TakeBufferHeader(int argCound, ...);
    int SendDataToServer(uint8_t *data);
    int TakerComRegisterSlaveData(uint8_t *SendData, string masterID);
    int TakerComBinlogDumpData(uint8_t *SendData);
    int TakerComBinlogDumpGTIDData(uint8_t *SendData);
    int TakerComQueryAndPrepareData(uint8_t *SendData, const string& command, const char *query);
    int TakerComQuitAndPingData(uint8_t *SendData, const string& command);
    int readBinlogResultSetPacket(char * RecvBuffer);

    int takerBufferHeaderSize(uint8_t * data, size_t *size, vector<uint8_t> *eventsData);


};
std::vector<std::string> getLocalIPs();

#endif //MYSQL_REPLICATER_SEND_H