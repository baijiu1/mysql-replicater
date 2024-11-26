//
// Created by 白杰 on 2024/4/22.
//

#include "packet.h"
#include <string>
#include <iostream>
#include <ios>
#include "config.h"


struct mysql_server_info MySQLServInfo;
struct mysql_plugin_name MySQLPluginName;
struct mysql_client_send_info MySQLClientSendInfo;
extern struct GlobalVar globalVar;

int Packet::Init() {
    MySQLServInfo.ProtocolLen = 0;
    MySQLServInfo.PacketBody = NULL;
    MySQLServInfo.SeqID = 0;
    MySQLServInfo.MySQLServInfoData.AuthPluginDataLenOr0x00 = NULL;
    MySQLServInfo.MySQLServInfoData.ProtocolVersion = '\0';
    MySQLServInfo.MySQLServInfoData.CharacterSet = '\0';
    MySQLServInfo.MySQLServInfoData.Reserved = NULL;
    MySQLServInfo.MySQLServInfoData.AuthPluginDataLen = '\0';
    MySQLServInfo.MySQLServInfoData.ProtocolVersion = '\0';
    MySQLServInfo.MySQLServInfoData.AuthPluginName = "";
    MySQLPluginName.cachingSha2Password = "caching_sha2_password";
    MySQLPluginName.mysqlClearPassword = "mysql_clear_password";
    MySQLPluginName.mysqlNativePassword = "mysql_native_password";
    MySQLPluginName.mysqlOldPassword = "mysql_old_password";
    MySQLPluginName.sha256Password = "sha256_password";
    MySQLClientSendInfo.ProtocolLen = 0;
    MySQLClientSendInfo.SeqID = 0;
//    MySQLClientSendInfo.PacketBody = NULL;
    fill(MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1, MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags1 + 4, ' ');
//    fill(MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags2, MySQLClientSendInfo.MySQLClientSendInfoData.CapabilityFlags2 + 4, ' ');
    fill(MySQLClientSendInfo.MySQLClientSendInfoData.MaxPacketLen, MySQLClientSendInfo.MySQLClientSendInfoData.MaxPacketLen + 1, ' ');
    fill(MySQLClientSendInfo.MySQLClientSendInfoData.CharacterSet, MySQLClientSendInfo.MySQLClientSendInfoData.CharacterSet + 2, ' ');
    fill(MySQLClientSendInfo.MySQLClientSendInfoData.Filler, MySQLClientSendInfo.MySQLClientSendInfoData.Filler + 23, 0x00);
    MySQLClientSendInfo.MySQLClientSendInfoData.UserName = globalVar.MySQL_Username;
//    MySQLClientSendInfo.MySQLClientSendInfoData.AuthData = "";
    MySQLClientSendInfo.MySQLClientSendInfoData.DataBaseName = "";
    MySQLServInfo.MySQLServInfoData.CapabilityFlags3 = 0;
    MySQLServInfo.MySQLServInfoData.ServerStatus1 = 0;
    MySQLServInfo.MySQLServInfoData.ServerAuthDataPartAll.clear();
};

Packet::Packet () {

}

int Packet::Parser_packet(unsigned char *buff) {
    if (buff[0] == 0xff) {
        return 1;
    }
    if (buff[0] < 10 ) {
        return 1;
    }
    int ProtocolLength = stoi(to_string(buff[0]), nullptr, 10);
    // + 2是因为协议头里，长度和seqid占两个字节
    MySQLServInfo.ProtocolLen = ProtocolLength + 2;
    printf("\n ProtocolLen: %d \n", MySQLServInfo.ProtocolLen);
//    printf("\n ProtocolLen buff: %#x \n", buff[0]);
    int SeqID = stoi(to_string(buff[3]), nullptr, 10);
    MySQLServInfo.SeqID = SeqID;
    MySQLServInfo.PacketBody = buff;
    int pos = 4;

    MySQLServInfo.MySQLServInfoData.ProtocolVersion = MySQLServInfo.PacketBody[pos];
    pos += 1;
    for (int k = pos; k <= MySQLServInfo.ProtocolLen; ++k) {
        if (MySQLServInfo.PacketBody[k] == 0) {
            break;
        }
        MySQLServInfo.MySQLServInfoData.ServerVersion.push_back(MySQLServInfo.PacketBody[k]);
        pos += 1;
    }
    pos += 1; // 12
//    printf("pos2: %d ", pos);
//    printf("MySQLServInfo.PacketBodytBody[pos]: %d ", MySQLServInfo.PacketBody[pos]);
    MySQLServInfo.MySQLServInfoData.ServerThreadID.push_back(MySQLServInfo.PacketBody[pos]);
    pos += 4;
//    printf("pos3: %d ", pos);
    printf("\n ServerAuthDataPart1 \n");
    for (int k = pos; k <= MySQLServInfo.ProtocolLen; ++k) {
        if (MySQLServInfo.PacketBody[k] == 0) {
            break;
        }
        printf(" %#x ", MySQLServInfo.PacketBody[k]);
        MySQLServInfo.MySQLServInfoData.ServerAuthDataPartAll.push_back(MySQLServInfo.PacketBody[k]);
        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart1 += string (1, MySQLServInfo.PacketBody[k]);
//        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart1.push_back(MySQLServInfo.PacketBody[k]);
        pos += 1;
    };
    printf("\n ServerAuthDataPart1 length: %lu \n", strlen(MySQLServInfo.MySQLServInfoData.ServerAuthDataPart1.c_str()));


//    printf("pos5: %d ", pos); //24
    if ( !MySQLServInfo.MySQLServInfoData.Filler.empty() ) {
        MySQLServInfo.MySQLServInfoData.Filler.clear();
    }
    MySQLServInfo.MySQLServInfoData.Filler.push_back(MySQLServInfo.PacketBody[pos]);
    printf(" %#x ", MySQLServInfo.MySQLServInfoData.Filler[0]);
    pos += 1; // 25
//    printf("pos6: %d ", pos);

    MySQLServInfo.MySQLServInfoData.CapabilityFlags1.push_back(MySQLServInfo.PacketBody[pos]);

    MySQLServInfo.MySQLServInfoData.CapabilityFlags3 |= MySQLServInfo.PacketBody[pos] & 0b11111111;
    pos += 1; // 26
    MySQLServInfo.MySQLServInfoData.CapabilityFlags3 |= (MySQLServInfo.PacketBody[pos] & 0b11111111) << 8;

    MySQLServInfo.MySQLServInfoData.CapabilityFlags1.push_back(MySQLServInfo.PacketBody[pos]);
    MySQLServInfo.MySQLServInfoData.CapabilityFlags.push_back((uint16_t)MySQLServInfo.MySQLServInfoData.CapabilityFlags1[0] | (uint16_t)MySQLServInfo.MySQLServInfoData.CapabilityFlags1[1]<<8);
    if (0 == (MySQLServInfo.MySQLServInfoData.CapabilityFlags[0]&CLIENT_PROTOCOL_41)) {
        return 1;
    }
    pos += 1; //27
    MySQLServInfo.MySQLServInfoData.CharacterSet = MySQLServInfo.PacketBody[pos];
    printf("CharacterSet: %#x \n", MySQLServInfo.MySQLServInfoData.CharacterSet);
    pos += 1; //28 status 0x2
//    printf("pos7: %d ", pos);
    MySQLServInfo.MySQLServInfoData.ServerStatus.push_back(MySQLServInfo.PacketBody[pos]);
    MySQLServInfo.MySQLServInfoData.ServerStatus1 |= MySQLServInfo.PacketBody[pos] & 0b11111111;
    pos += 1; //29 status 0x0
    MySQLServInfo.MySQLServInfoData.ServerStatus1 |= (MySQLServInfo.PacketBody[pos] & 0b11111111) << 8;
    MySQLServInfo.MySQLServInfoData.ServerStatus.push_back(MySQLServInfo.PacketBody[pos]);
    pos += 1; // 30
    MySQLServInfo.MySQLServInfoData.CapabilityFlags3 |= (MySQLServInfo.PacketBody[pos] & 0b11111111) << 16;
    MySQLServInfo.MySQLServInfoData.CapabilityFlags2.push_back(MySQLServInfo.PacketBody[pos]);
    pos += 1; // 31
    MySQLServInfo.MySQLServInfoData.CapabilityFlags3 |= (MySQLServInfo.PacketBody[pos] & 0b11111111) << 24;
    MySQLServInfo.MySQLServInfoData.CapabilityFlags2.push_back(MySQLServInfo.PacketBody[pos]);
//    printf("pos8: %d ", pos);
//
    pos += 1; // 32 0x15
    MySQLServInfo.MySQLServInfoData.AuthPluginDataLen = MySQLServInfo.PacketBody[pos];
    pos += 11; // 43
//    printf("pos9: %d ", pos);
    if ( !MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.empty() ) {
        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.clear();
    }
    printf("\n ServerAuthDataPart2 \n");
    for (int k = pos; k <= MySQLServInfo.ProtocolLen; ++k) {
        if (MySQLServInfo.PacketBody[k] == 0) {
            break;
        }
        printf(" %#x ", MySQLServInfo.PacketBody[k]);
        MySQLServInfo.MySQLServInfoData.ServerAuthDataPartAll.push_back(MySQLServInfo.PacketBody[k]);
        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.push_back(MySQLServInfo.PacketBody[k]);
        pos += 1;
    }
    printf("\n ServerAuthDataPart2 length: %lu \n", MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.size());
    pos += 1; // 56
//    printf("pos10: %d \n", pos); // 56

    if (MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.back() == 0) {
        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.pop_back();
    }
    for (int k = pos; k <= MySQLServInfo.ProtocolLen; ++k) {
        if (MySQLServInfo.PacketBody[k] == 0) {
            break;
        }
        MySQLServInfo.MySQLServInfoData.AuthPluginName += string(1, MySQLServInfo.PacketBody[pos]);
        pos += 1;
    }
    if (strcmp(MySQLServInfo.MySQLServInfoData.AuthPluginName.c_str(), "") == 0) {
        MySQLServInfo.MySQLServInfoData.AuthPluginName = defaultAuthPlugin;
    }
//    printf(" %s ", MySQLServInfo.MySQLServInfoData.AuthPluginName.c_str());
    return 0;
}

int Packet::Parser_packet_test(char *buff) {
    char header[4] = {0};
    uint32_t payloadLength = 0;
    for(int i = 0; i < 3; i++) {
        // 1001010
        payloadLength |= (buff[i] << (8 * i)) & 0b11111111;

    }
    printf("%#x", payloadLength);




//    if (buff[0] == 0xff) {
//        return 1;
//    }
//    if (buff[0] < 10 ) {
//        return 1;
//    }
//
//    int ProtocolLength = stoi(to_string(buff[0]), nullptr, 10);
//    // + 2是因为协议头里，长度和seqid占两个字节
//    MySQLServInfo.ProtocolLen = ProtocolLength + 2;
////    for (int i = 0; i < MySQLServInfo.ProtocolLen; ++i) {
////        printf(" %#x ", buff[i]);
////    }
//    printf("\n ProtocolLen: %d \n", MySQLServInfo.ProtocolLen);
////    printf("\n ProtocolLen buff: %#x \n", buff[0]);
//    int SeqID = stoi(to_string(buff[3]), nullptr, 10);
//    MySQLServInfo.SeqID = SeqID;
//    MySQLServInfo.PacketBody = buff;
//    int pos = 4;
//    for (int k = 4; k <= MySQLServInfo.ProtocolLen; ++k) {
//        printf(" %#x ", MySQLServInfo.PacketBody[k]);
//    }
//    MySQLServInfo.MySQLServInfoData.ProtocolVersion = MySQLServInfo.PacketBody[pos];
//    pos += 1;
////    printf("pos1: %d", pos);
//    for (int k = pos; k <= MySQLServInfo.ProtocolLen; ++k) {
//        if (MySQLServInfo.PacketBody[k] == 0) {
//            break;
//        }
//        MySQLServInfo.MySQLServInfoData.ServerVersion.push_back(MySQLServInfo.PacketBody[k]);
//        pos += 1;
//    }
//
//    pos += 1; // 12
////    printf("pos2: %d ", pos);
////    printf("MySQLServInfo.PacketBody[pos]: %d ", MySQLServInfo.PacketBody[pos]);
//    printf("\n 11: %#x \n", MySQLServInfo.PacketBody[pos]);
//    MySQLServInfo.MySQLServInfoData.ServerThreadID.push_back(MySQLServInfo.PacketBody[pos]);
//    pos += 4;
////    printf("pos3: %d ", pos);
//    for (int k = pos; k <= MySQLServInfo.ProtocolLen; ++k) {
//        if (MySQLServInfo.PacketBody[k] == 0) {
//            break;
//        }
//        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart1 += string(1, MySQLServInfo.PacketBody[k]);
////        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart1.push_back(MySQLServInfo.PacketBody[k]);
//        pos += 1;
//    };
//
////    printf("pos5: %d ", pos); //24
//    if ( !MySQLServInfo.MySQLServInfoData.Filler.empty() ) {
//        MySQLServInfo.MySQLServInfoData.Filler.clear();
//    }
//    MySQLServInfo.MySQLServInfoData.Filler.push_back(MySQLServInfo.PacketBody[pos]);
//    printf(" %#x ", MySQLServInfo.MySQLServInfoData.Filler[0]);
//    pos += 1; // 25
////    printf("pos6: %d ", pos);
//
//    MySQLServInfo.MySQLServInfoData.CapabilityFlags1.push_back(MySQLServInfo.PacketBody[pos]);
//    pos += 1; // 26
//    MySQLServInfo.MySQLServInfoData.CapabilityFlags1.push_back(MySQLServInfo.PacketBody[pos]);
//    MySQLServInfo.MySQLServInfoData.CapabilityFlags.push_back((uint16_t)MySQLServInfo.MySQLServInfoData.CapabilityFlags1[0] | (uint16_t)MySQLServInfo.MySQLServInfoData.CapabilityFlags1[1]<<8);
//    if (0 == (MySQLServInfo.MySQLServInfoData.CapabilityFlags[0]&CLIENT_PROTOCOL_41)) {
//        return 1;
//    }
//    pos += 1; //27
//    MySQLServInfo.MySQLServInfoData.CharacterSet = MySQLServInfo.PacketBody[pos];
//    printf("CharacterSet: %#x \n", MySQLServInfo.MySQLServInfoData.CharacterSet);
//    pos += 1; //28 status 0x2
////    printf("pos7: %d ", pos);
//    MySQLServInfo.MySQLServInfoData.ServerStatus.push_back(MySQLServInfo.PacketBody[pos]);
//    pos += 1; //29 status 0x0
//    MySQLServInfo.MySQLServInfoData.ServerStatus.push_back(MySQLServInfo.PacketBody[pos]);
//    pos += 1; // 30
//    MySQLServInfo.MySQLServInfoData.CapabilityFlags2.push_back(MySQLServInfo.PacketBody[pos]);
//    pos += 1; // 31
//    MySQLServInfo.MySQLServInfoData.CapabilityFlags2.push_back(MySQLServInfo.PacketBody[pos]);
////    printf("pos8: %d ", pos);
////
//    pos += 1; // 32 0x15
//    MySQLServInfo.MySQLServInfoData.AuthPluginDataLen = MySQLServInfo.PacketBody[pos];
//    pos += 11; // 43
////    printf("pos9: %d ", pos);
//    if ( !MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.empty() ) {
//        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.clear();
//    }
//    for (int k = pos; k <= MySQLServInfo.ProtocolLen; ++k) {
//        if (MySQLServInfo.PacketBody[k] == 0) {
//            break;
//        }
//        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.push_back(MySQLServInfo.PacketBody[k]);
//        pos += 1;
//    }
//    pos += 1; // 56
////    printf("pos10: %d \n", pos); // 56
//
//    if (MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.back() == 0) {
//        MySQLServInfo.MySQLServInfoData.ServerAuthDataPart2.pop_back();
//    }
//    for (int k = pos; k <= MySQLServInfo.ProtocolLen; ++k) {
//        if (MySQLServInfo.PacketBody[k] == 0) {
//            break;
//        }
//        MySQLServInfo.MySQLServInfoData.AuthPluginName += string(1, MySQLServInfo.PacketBody[pos]);
//        pos += 1;
//    }
//    if (strcmp(MySQLServInfo.MySQLServInfoData.AuthPluginName.c_str(), "") == 0) {
//        MySQLServInfo.MySQLServInfoData.AuthPluginName = defaultAuthPlugin;
//    }
//    printf(" %s ", MySQLServInfo.MySQLServInfoData.AuthPluginName.c_str());
    return 0;
}