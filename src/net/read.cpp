//
// Created by 白杰 on 2024/4/22.
//

#include "read.h"
#include "packet.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <netdb.h>
#include <cstring>
#include "config.h"
#include <fcntl.h>


using namespace std;
class Packet *pck;
struct mysql_conn_info MySQLConnInfo;
extern struct GlobalVar globalVar;

int Read::Connect() {
//    int fd = 0;
    MySQLConnInfo.connName = "";
    MySQLConnInfo.fd = 0;
    struct sockaddr_in serveraddr;

    MySQLConnInfo.fd = socket(AF_INET, SOCK_STREAM, 0);

//    char host_name[255];
//    std::string local_IP = "";
//
//    if (gethostname(host_name, sizeof(host_name)) == 0) {
//        struct hostent* host_info = gethostbyname(host_name);
//        if (host_info != nullptr) {
//            struct in_addr* address = (struct in_addr*)host_info->h_addr_list[0];
//            local_IP = inet_ntoa(*address);
//        }
//    }

    serveraddr.sin_family = AF_INET;
//    inet_pton(AF_INET, local_IP.c_str(), &serveraddr.sin_addr);
    inet_pton(AF_INET, globalVar.MySQL_Host.c_str(), &serveraddr.sin_addr);
    serveraddr.sin_port = htons(globalVar.MySQL_Port);

    MySQLConnInfo.serverAddr = serveraddr;

    int flags = fcntl(MySQLConnInfo.fd, F_GETFL, 0);
    fcntl(MySQLConnInfo.fd, F_SETFL, flags & ~O_NONBLOCK); // 设置为阻塞模式

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (-1 == setsockopt(MySQLConnInfo.fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval))) {
        printf("setsockopt failed!");
        exit(1);
    }


    int ret = connect(MySQLConnInfo.fd, (struct sockaddr *)&MySQLConnInfo.serverAddr, sizeof(MySQLConnInfo.serverAddr));
    if(ret == -1){
        printf("socket连接失败 \n");
        return 1;
    }
    unsigned char buff[120];
    uint8_t ServData[120];
    char dat[120];

    int len = recv(MySQLConnInfo.fd, buff, sizeof(buff), 0);
//    int len = recv(MySQLConnInfo.fd, dat, sizeof(ServData), 0);
//    printf("sizeof(ServData): %d \n", sizeof(ServData));
    if(len > 0){
        printf("\n server greeting: \n");
        for (int k = 0; k <= 119; ++k) {
            printf(" %#x ", buff[k]);
        }
        // resolve packet
        pck->Init();
//        pck->Parser_packet_test(dat);
        pck->Parser_packet(buff);

    }else if(len == 0){
        printf("服务器已断开连接...\n");
//        close(fd);
    }else{
        //到这里说明函数调用失败
        perror("recv error");
//        close(fd);
    }

//    close(fd);
    return 0;
}

Read::Read() {

}

int Read::Init() {
    Connect();
    return 0;
}