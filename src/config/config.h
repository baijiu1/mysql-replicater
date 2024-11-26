//
// Created by 白杰 on 2024/4/22.
//

#ifndef MYSQL_REPLICATER_CONFIG_H
#define MYSQL_REPLICATER_CONFIG_H


#define     CLIENT_LONG_PASSWORD   1
#define 	CLIENT_FOUND_ROWS   2
#define 	CLIENT_LONG_FLAG   4
#define 	CLIENT_CONNECT_WITH_DB   8
#define 	CLIENT_NO_SCHEMA    16
#define 	CLIENT_COMPRESS   32
#define 	CLIENT_ODBC   64
#define 	CLIENT_LOCAL_FILES   128
#define 	CLIENT_IGNORE_SPACE   256
#define     CLIENT_PROTOCOL_41   512
#define 	CLIENT_INTERACTIVE   1024
#define 	CLIENT_SSL   2048
#define 	CLIENT_IGNORE_SIGPIPE   4096
#define 	CLIENT_TRANSACTIONS   8192
#define 	CLIENT_RESERVED   16384
#define 	CLIENT_RESERVED2    32768
#define 	CLIENT_MULTI_STATEMENTS   (1UL << 16)
#define 	CLIENT_MULTI_RESULTS   (1UL << 17)
#define 	CLIENT_PS_MULTI_RESULTS   (1UL << 18)
#define 	CLIENT_PLUGIN_AUTH   (1UL << 19)
#define 	CLIENT_CONNECT_ATTRS   (1UL << 20)
#define 	CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA   (1UL << 21)
#define 	CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS   (1UL << 22)
#define 	CLIENT_SESSION_TRACK   (1UL << 23)
#define 	CLIENT_DEPRECATE_EOF   (1UL << 24)
#define 	CLIENT_OPTIONAL_RESULTSET_METADATA   (1UL << 25)
#define 	CLIENT_ZSTD_COMPRESSION_ALGORITHM   (1UL << 26)
#define 	CLIENT_QUERY_ATTRIBUTES   (1UL << 27)
#define 	MULTI_FACTOR_AUTHENTICATION   (1UL << 28)
#define 	CLIENT_CAPABILITY_EXTENSION   (1UL << 29)
#define 	CLIENT_SSL_VERIFY_SERVER_CERT   (1UL << 30)
#define 	CLIENT_REMEMBER_OPTIONS   (1UL << 31)
#define     MAXPACKETSIZE   16777216

#define      COM_SLEEP           0x00
#define      COM_QUIT           0x01
#define      COM_INIT_DB           0x02
#define      COM_QUERY           0x03
#define      COM_FIELD_LIST           0x04
#define      COM_CREATE_DB           0x05
#define      COM_DROP_DB           0x06
#define      COM_REFRESH           0x07
#define      COM_SHUTDOWN           0x08
#define      COM_STATISTICS           0x09
#define      COM_PROCESS_INFO           0x0A
#define      COM_CONNECT           0x0B
#define      COM_PROCESS_KILL           0x0C
#define      COM_DEBUG           0x0D
#define      COM_PING           0x0E
#define      COM_TIME           0x0F
#define      COM_DELAYED_INSERT           0x10
#define      COM_CHANGE_USER           0x11
#define      COM_BINLOG_DUMP           0x12
#define      COM_TABLE_DUMP           0x13
#define      COM_CONNECT_OUT           0x14
#define      COM_REGISTER_SLAVE           0x15
#define      COM_STMT_PREPARE           0x16
#define      COM_STMT_EXECUTE           0x17
#define      COM_STMT_SEND_LONG_DATA           0x18
#define      COM_STMT_CLOSE           0x19
#define      COM_STMT_RESET           0x1A
#define      COM_SET_OPTION           0x1B
#define      COM_STMT_FETCH           0x1C

#define CLIENT_BASIC_FLAGS                                          \
  (CLIENT_ALL_FLAGS &                                               \
   ~(CLIENT_SSL | CLIENT_COMPRESS | CLIENT_SSL_VERIFY_SERVER_CERT | \
     CLIENT_ZSTD_COMPRESSION_ALGORITHM))

#define CLIENT_ALL_FLAGS                                                       \
  (CLIENT_LONG_PASSWORD | CLIENT_FOUND_ROWS | CLIENT_LONG_FLAG |               \
   CLIENT_CONNECT_WITH_DB | CLIENT_NO_SCHEMA | CLIENT_COMPRESS | CLIENT_ODBC | \
   CLIENT_LOCAL_FILES | CLIENT_IGNORE_SPACE | CLIENT_PROTOCOL_41 |             \
   CLIENT_INTERACTIVE | CLIENT_SSL | CLIENT_IGNORE_SIGPIPE |                   \
   CLIENT_TRANSACTIONS | CLIENT_RESERVED | CLIENT_RESERVED2 |                  \
   CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS | CLIENT_PS_MULTI_RESULTS |  \
   CLIENT_SSL_VERIFY_SERVER_CERT | CLIENT_REMEMBER_OPTIONS |                   \
   CLIENT_PLUGIN_AUTH | CLIENT_CONNECT_ATTRS |                                 \
   CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA |                                     \
   CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS | CLIENT_SESSION_TRACK |                \
   CLIENT_DEPRECATE_EOF | CLIENT_OPTIONAL_RESULTSET_METADATA |                 \
   CLIENT_ZSTD_COMPRESSION_ALGORITHM | CLIENT_QUERY_ATTRIBUTES |               \
   MULTI_FACTOR_AUTHENTICATION)

#define defaultAuthPlugin "mysql_native_password";

#include <string>

using namespace std;


class ReplicaterConfig {
public:
    string MySQL_Username;
    string MySQL_Password;
    string MySQL_Host;
    string MySQL_Port;
    string MySQL_BinlogName;
    string MySQL_BinlogPos;
    string uuid;
    string Log_File;
    string Log_Level;
    string Data_dir;
};

struct GlobalVar {
    int MySQL_ID;
    string MySQL_Username;
    string MySQL_Password;
    string MySQL_Host;
    int MySQL_Port;
    uint64_t Log_Pos;
    string MySQL_DataBaseName;
    string MySQL_CharacterSet;
    string MySQL_BinlogName;
    string MySQL_BinlogPos;
    string uuid;
    string Log_File;
    string Log_Level;
    string Data_dir;
    string MySQL_AllowAllFiles;
    string MySQL_AllowCleartextPasswords;
    string MySQL_AllowFallbackToPlaintext;
    string MySQL_AllowNativePasswords;
    string MySQL_AllowOldPasswords;
    string MySQL_CheckConnLiveness;
    string MySQL_ClientFoundRows;
    string MySQL_ColumnsWithAlias;
    string MySQL_InterpolateParams;
    string MySQL_MultiStatements;
    string MySQL_ParseTime;
    string MySQL_RejectReadOnly;
    string MySQL_TLS;
    string MySQL_Collation;
};

int MapSet();

#endif //MYSQL_REPLICATER_CONFIG_H