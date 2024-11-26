//
// Created by 白杰 on 2024/4/22.
//

#ifndef MYSQL_REPLICATER_AUTH_H
#define MYSQL_REPLICATER_AUTH_H


#include <cstdint>

using namespace std;

class Auth {
public:
    Auth();
    struct mysql_conn_info {

    };
    int Close();
    int Init();
    uint8_t * appendLengthEncodedInteger(uint8_t authRespLEIBuf[9], const int authRespLen);
    int writePacket(uint8_t *data, int32_t pktLen);
    int AuthenticationSwitchRequest(char recvbuffer[60]);

    void MySQLCachingSha2Password(const char *password, const char *AuthDataPart, unsigned char* SHA256digest);

    void MysqlNativePassword_shaOther(const char *password, const char *AuthDataPart, unsigned char *SHA1digest);
};

#endif //MYSQL_REPLICATER_AUTH_H