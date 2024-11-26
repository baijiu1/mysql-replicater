//
// Created by 白杰 on 2024/4/22.
//

#include "auth.h"
#include "packet.h"
#include "config.h"
#include <openssl/sha.h>
#include <string>
#include <map>
#include <cstring>
#include <unistd.h>

using namespace std;

extern struct mysql_server_info MySQLServInfo;
//extern class ReplicaterConfig GlobalVar;
extern struct GlobalVar globalVar;
extern struct mysql_plugin_name MySQLPluginName;
extern struct mysql_client_send_info MySQLClientSendInfo;
extern struct mysql_conn_info MySQLConnInfo;

unsigned char stage0[SHA256_DIGEST_LENGTH];
unsigned char stage1[SHA256_DIGEST_LENGTH];
unsigned char stage2[SHA256_DIGEST_LENGTH];
unsigned char stage3[SHA256_DIGEST_LENGTH];
unsigned char stageAll[SHA256_DIGEST_LENGTH];





int Auth::Init() {
    // 准备好所有数据
    printf("\n");
    if (strcmp(MySQLServInfo.MySQLServInfoData.AuthPluginName.c_str(), MySQLPluginName.cachingSha2Password) == 0) {
        printf("\nusing cachingSha2Password auth plugin\n");
        for (int i = 0; i < MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.size(); ++i) {
            MySQLServInfo.MySQLServInfoData.ServerAuthDataPart += MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2[i];
        }
        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart += MySQLServInfo.MySQLServInfoData.ServerAuthDataPart1;

        unsigned char SHA256digest[SHA256_DIGEST_LENGTH];
        MySQLCachingSha2Password(globalVar.MySQL_Password.c_str(), MySQLServInfo.MySQLServInfoData.ServerAuthDataPart.c_str(), SHA256digest);
        MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA.resize(SHA256_DIGEST_LENGTH);
        memcpy(MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA.data(), SHA256digest, sizeof(SHA256digest));

    } else if (strcmp(MySQLServInfo.MySQLServInfoData.AuthPluginName.c_str(), MySQLPluginName.mysqlNativePassword) == 0) {
        printf("\nusing mysqlNativePassword auth plugin\n");
        for (int i = 0; i < MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.size(); ++i) {
            MySQLServInfo.MySQLServInfoData.ServerAuthDataPart += MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2[i];
        }
        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart += MySQLServInfo.MySQLServInfoData.ServerAuthDataPart1;
        unsigned char SHA1digest[SHA_DIGEST_LENGTH];
        MysqlNativePassword_shaOther(globalVar.MySQL_Password.c_str(), MySQLServInfo.MySQLServInfoData.ServerAuthDataPart.c_str(), SHA1digest);
        MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA.resize(SHA_DIGEST_LENGTH);
        memcpy(MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA.data(), SHA1digest, sizeof(SHA1digest));
    } else {
        printf("unknown auth plugin: %s", MySQLServInfo.MySQLServInfoData.AuthPluginName.c_str());
        return 0;
    }
    // CapabilityFlags1
    MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1[0] = CLIENT_PROTOCOL_41 | CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS | CLIENT_PS_MULTI_RESULTS
            | CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS | CLIENT_DEPRECATE_EOF | CLIENT_INTERACTIVE
            | CLIENT_SESSION_TRACK | CLIENT_CONNECT_WITH_DB | CLIENT_RESERVED2
            | CLIENT_LONG_PASSWORD | CLIENT_TRANSACTIONS | CLIENT_LOCAL_FILES | CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA
            | CLIENT_PLUGIN_AUTH | MySQLServInfo.MySQLServInfoData.CapabilityFlags[0]&CLIENT_LONG_FLAG;
    if (strcmp(globalVar.MySQL_ClientFoundRows.c_str(), "true") == 0) {
        MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1[0] |= CLIENT_FOUND_ROWS;
    }
    if (strcmp(globalVar.MySQL_TLS.c_str(), "true") == 0) {
        MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1[0] |= CLIENT_SSL;
    }
//    if (strcmp(globalVar.MySQL_MultiStatements.c_str(), "true") == 0) {
//        MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1[0] |= CLIENT_MULTI_STATEMENTS;
//    }
    const int authRespLen = sizeof(stage3);
    uint8_t authRespLEIBuf[9];
    uint8_t* authRespLEI = appendLengthEncodedInteger(authRespLEIBuf, authRespLen);
    printf(" authRespLEI[0]: %d \n", authRespLEI[0]);
    printf(" authRespLen: %lu \n", sizeof(stage3));
//    if (sizeof(authRespLEI) > 1) {
//        // (uint8_t)2097152
//        MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1[0] |= CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA;
//    }
//    if (strlen(MySQLClientSendInfo.MySQLClientSendInfoData.DataBaseName.c_str()) >= 1) {
//        MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1[0] |= CLIENT_CONNECT_WITH_DB;
//    }

    MySQLClientSendInfo.MySQLClientSendInfoData.UserName = globalVar.MySQL_Username;
    MySQLClientSendInfo.MySQLClientSendInfoData.DataBaseName = globalVar.MySQL_DataBaseName;
    MySQLClientSendInfo.MySQLClientSendInfoData.ClientPluginName = MySQLServInfo.MySQLServInfoData.AuthPluginName;

    // 4: capabilityFlags + 4: maxpacketLen + 1: CharacterSet + 23: Fillter 0x00 + UserName.length() + 1: 0x00
    // + authRespLEI[0] = 1(内容为32) + AuthResp = 32 + databasename.length() + 1: 0x00 + pluginname.length() 21 + 4:报文头的4位
//    pktLen = 4 + 4 + 1 + 23 + strlen(MySQLClientSendInfo.MySQLClientSendInfoData.UserName.c_str()) + 1 + authRespLEI[0] + 1 + 21 + 1 + 4;
//    pktLen = 4+  4 + 4 + 1 + 23 + strlen(MySQLClientSendInfo.MySQLClientSendInfoData.UserName.c_str()) + 1 + 21 + authRespLEI[0] + 1 +
//  uint32_t responsePayloadLen = 4+4+4+1+23+strlen(username)+1+21+strlen(database)+1+strlen(responsePluginName)+1+1;
// 33: 32 + 1 32:sha256数据长度 1:信息长度
    const int pktLen = 4 + 4 + 4 + 1 + 23 + strlen(MySQLClientSendInfo.MySQLClientSendInfoData.UserName.c_str()) + 1 + MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA.size() +
                       strlen(MySQLClientSendInfo.MySQLClientSendInfoData.DataBaseName.c_str()) + 1 + strlen(MySQLClientSendInfo.MySQLClientSendInfoData.ClientPluginName.c_str()) + 1;
    uint8_t data[pktLen];

    printf("MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA.size(): %lu", MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA.size());
    MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlagsAll = CLIENT_BASIC_FLAGS;
    MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlagsAll1[0] = static_cast<uint8_t>(CLIENT_PROTOCOL_41 | CLIENT_LONG_PASSWORD | CLIENT_LONG_FLAG
            | CLIENT_LOCAL_FILES | CLIENT_INTERACTIVE | CLIENT_TRANSACTIONS | CLIENT_PLUGIN_AUTH);

    MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlagsAll1[1] = static_cast<uint8_t>(CLIENT_INTERACTIVE | CLIENT_TRANSACTIONS | CLIENT_PLUGIN_AUTH);

    MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlagsAll2[0] = static_cast<uint8_t>(CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS | CLIENT_PS_MULTI_RESULTS
            | CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA | CLIENT_CONNECT_ATTRS | CLIENT_PLUGIN_AUTH | CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS
            | CLIENT_SESSION_TRACK | CLIENT_DEPRECATE_EOF | CLIENT_QUERY_ATTRIBUTES | MULTI_FACTOR_AUTHENTICATION);
    MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlagsAll2[1] = static_cast<uint8_t>(CLIENT_DEPRECATE_EOF | CLIENT_QUERY_ATTRIBUTES | MULTI_FACTOR_AUTHENTICATION);



    printf("\n data[0] length 93: %hhd \n", data[0]);
    data[3] = 0x01;
    int position = 4;

    data[position] = MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1[0];
    printf(" data[position]1: %#x ", data[position]);
    position += 1;
    data[position] = MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1[0] >> 8;
    printf(" data[position]2: %#x ", data[position]);
    position += 1;
    data[position] = MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1[0] >> 16;
    printf(" data[position]3: %#x ", data[position]);
    position += 1;
    data[position] = MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1[0] >> 24;
    printf(" data[position]4: %#x ", data[position]);
    position += 1;

    data[position] = 0x00;
    position += 1;
    data[position] = 0x00;
    position += 1;
    data[position] = 0x00;
    position += 1;
    data[position] = 0x00;
    position += 1;

            // position: 12
//        responsePayload[i+8] = (maxPacketSize >> (i * 8)) & 0b11111111;
//    MySQLClientSendInfo.PacketBody.push_back(MySQLServInfo.MySQLServInfoData.CharacterSet);
    data[position] = MySQLServInfo.MySQLServInfoData.CharacterSet;
    position += 1; // 13
//    responsePayload[12] = characterSet;
    for (int i = 0; i < 23; ++i) {
//        MySQLClientSendInfo.PacketBody.push_back(MySQLClientSendInfo.MySQLClientSendInfoData.Filler[i]);
        data[position] = MySQLClientSendInfo.MySQLClientSendInfoData.Filler[i];
        position += 1;
    }
    // 36
    for(int i = 0; i < strlen(MySQLClientSendInfo.MySQLClientSendInfoData.UserName.c_str()) + 1; i++) {
        if (MySQLClientSendInfo.MySQLClientSendInfoData.UserName[i] == 0) {
            break;
        }
        MySQLClientSendInfo.PacketBody.push_back(MySQLClientSendInfo.MySQLClientSendInfoData.UserName[i]);
        data[position] = MySQLClientSendInfo.MySQLClientSendInfoData.UserName[i];
        position += 1;
    }
    data[position] = 0x00;
    position += 1;
    data[position] = MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA.size();
    position += 1;
    for (int i = 0; i < MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA.size(); ++i) {
//        MySQLClientSendInfo.PacketBody.push_back(stageAll[i]);
        if (MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA[i] == 0) {
            break;
        }
        data[position] = MySQLClientSendInfo.MySQLClientSendInfoData.AuthRespSHA[i];
        position += 1;
    }
//    data[position] = 0;
//    position += 1;
//    data[position] = strlen(MySQLClientSendInfo.MySQLClientSendInfoData.DataBaseName.c_str());
//    position += 1;
//    MySQLClientSendInfo.PacketBody.push_back(0x20);
    for(int i = 0; i < strlen(MySQLClientSendInfo.MySQLClientSendInfoData.DataBaseName.c_str()); i++) {
//        if (MySQLClientSendInfo.MySQLClientSendInfoData.DataBaseName[i] == 0) {
//            break;
//        }
        data[position] = MySQLClientSendInfo.MySQLClientSendInfoData.DataBaseName[i];
        position += 1;
    }
    data[position] = 0x00;
    position += 1;
    for(int i = 0; i < strlen(MySQLServInfo.MySQLServInfoData.AuthPluginName.c_str()); i++) {
        if (MySQLServInfo.MySQLServInfoData.AuthPluginName[i] == 0) {
            break;
        }
        data[position] = MySQLServInfo.MySQLServInfoData.AuthPluginName[i];
        position += 1;
    }
    data[position] = 0x00;

    data[0] = pktLen - 4;
    data[1] = 0x00;
    data[2] = 0x00;

    writePacket(data, pktLen);
    return 0;

};

uint8_t * Auth::appendLengthEncodedInteger(uint8_t authRespLEIBuf[9], const int authRespLen) {
    if (authRespLen <= 250) {
        authRespLEIBuf[0] = (uint8_t)authRespLen;
        return authRespLEIBuf;
    } else if (authRespLen <= 0xffff) {
        // 0xfc, byte(n), byte(n>>8)
        authRespLEIBuf[0] = 0xfc;
        authRespLEIBuf[1] = authRespLen;
        authRespLEIBuf[2] = authRespLen >> 8;
        return authRespLEIBuf;
    } else if (authRespLen <= 0xffffff) {
        // 0xfd, byte(n), byte(n>>8), byte(n>>16)
        authRespLEIBuf[0] = 0xfd;
        authRespLEIBuf[1] = authRespLen;
        authRespLEIBuf[2] = authRespLen >> 8;
        authRespLEIBuf[2] = authRespLen >> 16;
        return authRespLEIBuf;
    } else {
        authRespLEIBuf[0] = 0xf3;
        authRespLEIBuf[1] = authRespLen;
        authRespLEIBuf[2] = authRespLen >> 8;
        authRespLEIBuf[3] = authRespLen >> 16;
        authRespLEIBuf[4] = authRespLen >> 24;
        authRespLEIBuf[5] = authRespLen >> 32;
        authRespLEIBuf[6] = authRespLen >> 40;
        authRespLEIBuf[7] = authRespLen >> 48;
        authRespLEIBuf[8] = authRespLen >> 56;
        return authRespLEIBuf;
    }
}


struct mysql_EOF_packet MySQLEOFPacket;
int Auth::writePacket(uint8_t *data, int32_t pktLen) {
    // connect to server, write data
    printf("\n data[num]: \n");
    printf("\n pktLen: %d \n", pktLen);
    for (int num = 0 ; num < pktLen; ++num) {
        printf(" %#x ", data[num]);
    }

//    size_t sendLen = send(MySQLConnInfo.fd, result, MySQLClientSendInfo.PacketBody.size(), 0);
    size_t sendLen = send(MySQLConnInfo.fd, data, pktLen, 0);
    if (sendLen > 0) {
        printf("\n auth ok! \n");
    } else {
        printf("send error \n");
    }

    char recvbuffer[60];
    int recvLen = recv(MySQLConnInfo.fd, recvbuffer, sizeof(recvbuffer), 0);
    if (recvLen > 0) {
        printf("\n recv ok!! \n");
        if (recvbuffer[4] == 0xfffffffe || recvbuffer[4] == 254 || recvbuffer[4] == 0xfe) {
            printf("server和client密码插件不一致，需要重新鉴权");
            AuthenticationSwitchRequest(recvbuffer);
        } else {
            printf("\n Response OK! \n");
        }
    }

    return 0;
}

int Auth::AuthenticationSwitchRequest(char recvbuffer[60]) {
    char *SwitchAuthCode;
    int AuthPos = 0;
    if (strcmp(MySQLServInfo.MySQLServInfoData.AuthPluginName.c_str(), MySQLPluginName.mysqlNativePassword) == 0) {
        AuthPos = 22 + 1 + 4;
        for (int i = AuthPos; i < sizeof(recvbuffer); ++i) {
            if (recvbuffer[i] == 0) {
                break;
            }
            SwitchAuthCode[i] = recvbuffer[i];
        }
        unsigned char SwitchAuthSHA1digest[SHA_DIGEST_LENGTH];
        MysqlNativePassword_shaOther(globalVar.MySQL_Password.c_str(), SwitchAuthCode, SwitchAuthSHA1digest);
        if (sizeof(SwitchAuthSHA1digest) > 1) {
            unsigned char SwitchAuthData[SHA_DIGEST_LENGTH + 4];
            SwitchAuthData[0] = 0x14;
            SwitchAuthData[1] = 0x00;
            SwitchAuthData[2] = 0x00;
            SwitchAuthData[3] = 0x03;
            for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
                SwitchAuthData[i+4] = SwitchAuthSHA1digest[i];
            }
            size_t sendSwitchAuthLen = send(MySQLConnInfo.fd, SwitchAuthData, sizeof(SwitchAuthData), 0);
            if (sendSwitchAuthLen > 1) {
                printf("\nSwitch Auth Response\n");
                char SwitchAuthRecv[40];
                int SwitchAuthRecvLen = recv(MySQLConnInfo.fd, SwitchAuthRecv, sizeof(SwitchAuthRecv), 0);
                if (SwitchAuthRecvLen > 1 && SwitchAuthRecv[4] == 0) {
                    printf("\nSwitch Auth Response OK!Begin start Command Phase\n");
                } else {
                    char recvbuffer[60];
                    AuthenticationSwitchRequest(recvbuffer);
                }
            }
        }
    } else if (strcmp(MySQLServInfo.MySQLServInfoData.AuthPluginName.c_str(), MySQLPluginName.cachingSha2Password) == 0) {

    } else {
        printf("unknown plugin");
        exit(1);
    }

    return 0;
}

void Auth::MySQLCachingSha2Password(const char *password, const char *AuthDataPart, unsigned char* SHA256digest) {
//    unsigned char stgAll[SHA256_DIGEST_LENGTH];
//        The server side plugin name is caching_sha2_password
//        The client side plugin name is caching_sha2_password
//        Account - user account (user-host combination)
//        authentication_string - Transformation of account password stored in mysql.user table
//        user_password - Password known to generate authentication_string for given user account
//        client_password - password used by client while connecting to server
//        Nonce - 20 byte long random data 两个随机挑战书拼接在一起的长度
//        Scramble - XOR(SHA256(password), SHA256(SHA256(SHA256(password)), Nonce))
//        Hash entry - account_name -> SHA256(SHA256(user_password))
    // first sha256 SHA256(password)
    SHA256_CTX ctx0;
    SHA256_Init(&ctx0);
    SHA256_Update(&ctx0, password, strlen(password));
    unsigned char md0[SHA256_DIGEST_LENGTH];
    SHA256_Final(md0, &ctx0);



    SHA256_CTX ctx1;
    SHA256_Init(&ctx1);
    SHA256_Update(&ctx1, password, strlen(password));
    unsigned char md1[SHA256_DIGEST_LENGTH];
    SHA256_Final(md1, &ctx1);

    SHA256_CTX ctx2;
    SHA256_Init(&ctx2);
    SHA256_Update(&ctx2, md1, sizeof(md1));
    unsigned char md2[SHA256_DIGEST_LENGTH];
    SHA256_Final(md2, &ctx2);
    unsigned char resultAll[52];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        resultAll[i] = MySQLServInfo.MySQLServInfoData.ServerAuthDataPartAll[i];
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        resultAll[i+20] = md2[i];
    }
    SHA256_CTX ctx3;
    SHA256_Init(&ctx3);
    SHA256_Update(&ctx3, resultAll, sizeof(resultAll));
    unsigned char md3[SHA256_DIGEST_LENGTH];
    SHA256_Final(md3, &ctx3);

    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        SHA256digest[i] = md0[i] ^ md3[i];
    }

//
//    SHA256((const unsigned char*) password, strlen(password), stg0);
//
//    // sha256 SHA256(password)
//    SHA256((const unsigned char*) password, strlen(password), stg1);
//    // SHA256(SHA256(password))
//    SHA256((const unsigned char*) stg1, sizeof(stg1)/sizeof(stg1[0]), stg2);
//    // stage2 + nonce
//    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
//        AuthDataPart += stg2[i];
//    }
//    // SHA256(SHA256(SHA256(password)), Nonce)
//    SHA256((const unsigned char*) AuthDataPart, strlen(AuthDataPart), stg3);
//    // XOR(SHA256(password), SHA256(SHA256(SHA256(password)), Nonce)) stage0 ^ stage3
//    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
//        SHA256digest[i] = stg0[i] ^ stg3[i];
//    }
//    printf("\nSHA256digest\n");
//    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
//        printf(" %#x ", SHA256digest[i]);
//    }
}

/*
@param
 $1: know of context of password
 $2: 20-byte random challenge
 $3: know of context of password of length
 $4: digest of final general
 */
void Auth::MysqlNativePassword_shaOther(const char *password, const char *AuthDataPart, unsigned char* SHA1digest) {
    SHA_CTX ctx1;
    SHA1_Init(&ctx1);
    SHA1_Update(&ctx1, password, strlen(password));
    unsigned char md1[SHA_DIGEST_LENGTH];
    SHA1_Final(md1, &ctx1);

    SHA_CTX ctx2;
    SHA1_Init(&ctx2);
    SHA1_Update(&ctx2, md1, sizeof(md1));
    unsigned char md2[SHA_DIGEST_LENGTH];
    SHA1_Final(md2, &ctx2);

    unsigned char resultAll[40];
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        resultAll[i] = MySQLServInfo.MySQLServInfoData.ServerAuthDataPartAll[i];
    }
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        resultAll[i+20] = md2[i];
    }

    SHA_CTX ctx3;
    SHA1_Init(&ctx3);
    SHA1_Update(&ctx3, resultAll, sizeof(resultAll));
    unsigned char md3[SHA_DIGEST_LENGTH];
    SHA1_Final(md3, &ctx3);

    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        SHA1digest[i] = md1[i] ^ md3[i];
    }

};