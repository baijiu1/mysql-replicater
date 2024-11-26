//
// Created by 白杰 on 2024/4/22.
//

#include "config.h"
#include <map>
#include "binlogEvent.h"

using namespace std;
std::map<Log_event_type, int> BinlogEventMap;
std::map<int, Log_event_type> BinlogEventIntMap;
std::map<std::string, uint8_t> CommandMap;

int MapSet() {
    CommandMap.insert(std::make_pair("COM_SLEEP",0x00));
    CommandMap.insert(std::make_pair("COM_QUIT",0x01));
    CommandMap.insert(std::make_pair("COM_INIT_DB",0x02));
    CommandMap.insert(std::make_pair("COM_QUERY",0x03));
    CommandMap.insert(std::make_pair("COM_FIELD_LIST",0x04));
    CommandMap.insert(std::make_pair("COM_CREATE_DB",0x05));
    CommandMap.insert(std::make_pair("COM_DROP_DB",0x06));
    CommandMap.insert(std::make_pair("COM_REFRESH",0x07));
    CommandMap.insert(std::make_pair("COM_SHUTDOWN",0x08));
    CommandMap.insert(std::make_pair("COM_STATISTICS",0x09));
    CommandMap.insert(std::make_pair("COM_PROCESS_INFO",0x0A));
    CommandMap.insert(std::make_pair("COM_CONNECT",0x0B));
    CommandMap.insert(std::make_pair("COM_PROCESS_KILL",0x0C));
    CommandMap.insert(std::make_pair("COM_DEBUG",0x0D));
    CommandMap.insert(std::make_pair("COM_PING",0x0E));
    CommandMap.insert(std::make_pair("COM_TIME",0x0F));
    CommandMap.insert(std::make_pair("COM_DELAYED_INSERT",0x10));
    CommandMap.insert(std::make_pair("COM_CHANGE_USER",0x11));
    CommandMap.insert(std::make_pair("COM_BINLOG_DUMP",0x12));
    CommandMap.insert(std::make_pair("COM_TABLE_DUMP",0x13));
    CommandMap.insert(std::make_pair("COM_CONNECT_OUT",0x14));
    CommandMap.insert(std::make_pair("COM_REGISTER_SLAVE",0x15));
    CommandMap.insert(std::make_pair("COM_STMT_PREPARE",0x16));
    CommandMap.insert(std::make_pair("COM_STMT_EXECUTE",0x17));
    CommandMap.insert(std::make_pair("COM_STMT_SEND_LONG_DATA",0x18));
    CommandMap.insert(std::make_pair("COM_STMT_CLOSE",0x19));
    CommandMap.insert(std::make_pair("COM_STMT_RESET",0x1A));
    CommandMap.insert(std::make_pair("COM_SET_OPTION",0x1B));
    CommandMap.insert(std::make_pair("COM_STMT_FETCH",0x1C));

    CommandMap.insert(std::make_pair("UNKNOWN_EVENT",0));
    CommandMap.insert(std::make_pair("START_EVENT_V3",1));
    CommandMap.insert(std::make_pair("QUERY_EVENT",2));
    CommandMap.insert(std::make_pair("STOP_EVENT",3));
    CommandMap.insert(std::make_pair("ROTATE_EVENT",4));
    CommandMap.insert(std::make_pair("INTVAR_EVENT",5));
    CommandMap.insert(std::make_pair("LOAD_EVENT",6));
    CommandMap.insert(std::make_pair("SLAVE_EVENT",7));
    CommandMap.insert(std::make_pair("CREATE_FILE_EVENT",8));
    CommandMap.insert(std::make_pair("APPEND_BLOCK_EVENT",9));
    CommandMap.insert(std::make_pair("EXEC_LOAD_EVENT",10));
    CommandMap.insert(std::make_pair("DELETE_FILE_EVENT",11));
    CommandMap.insert(std::make_pair("NEW_LOAD_EVENT",12));
    CommandMap.insert(std::make_pair("RAND_EVENT",13));
    CommandMap.insert(std::make_pair("USER_VAR_EVENT",14));
    CommandMap.insert(std::make_pair("FORMAT_DESCRIPTION_EVENT",15));
    CommandMap.insert(std::make_pair("XID_EVENT",16));
    CommandMap.insert(std::make_pair("BEGIN_LOAD_QUERY_EVENT",17));
    CommandMap.insert(std::make_pair("EXECUTE_LOAD_QUERY_EVENT",18));
    CommandMap.insert(std::make_pair("TABLE_MAP_EVENT",19));
    CommandMap.insert(std::make_pair("WRITE_ROWS_EVENTv0",20));
    CommandMap.insert(std::make_pair("UPDATE_ROWS_EVENTv0",21));
    CommandMap.insert(std::make_pair("DELETE_ROWS_EVENTv0",22));
    CommandMap.insert(std::make_pair("WRITE_ROWS_EVENTv1",23));
    CommandMap.insert(std::make_pair("UPDATE_ROWS_EVENTv1",24));
    CommandMap.insert(std::make_pair("DELETE_ROWS_EVENTv1",25));
    CommandMap.insert(std::make_pair("INCIDENT_EVENT",26));
    CommandMap.insert(std::make_pair("HEARTBEAT_EVENT",27));
    CommandMap.insert(std::make_pair("IGNORABLE_EVENT",28));
    CommandMap.insert(std::make_pair("ROWS_QUERY_EVENT",29));
    CommandMap.insert(std::make_pair("WRITE_ROWS_EVENTv2",30));
    CommandMap.insert(std::make_pair("UPDATE_ROWS_EVENTv2",31));
    CommandMap.insert(std::make_pair("DELETE_ROWS_EVENTv2",32));
    CommandMap.insert(std::make_pair("GTID_EVENT",33));
    CommandMap.insert(std::make_pair("ANONYMOUS_GTID_EVENT",34));
    CommandMap.insert(std::make_pair("PREVIOUS_GTIDS_EVENT",35));
    CommandMap.insert(std::make_pair("TRANSACTION_CONTEXT_EVENT",36));
    CommandMap.insert(std::make_pair("VIEW_CHANGE_EVENT",37));
    CommandMap.insert(std::make_pair("XA_PREPARE_LOG_EVENT",38));


    // binlog event header length
    BinlogEventMap.insert(std::make_pair( START_EVENT_V3 ,56));
    BinlogEventMap.insert(std::make_pair( QUERY_EVENT ,13));
    BinlogEventMap.insert(std::make_pair( STOP_EVENT ,0));
    BinlogEventMap.insert(std::make_pair( ROTATE_EVENT ,8));
    BinlogEventMap.insert(std::make_pair( INTVAR_EVENT ,0));
//    BinlogEventMap.insert(std::make_pair( LOAD_EVENT ,18));
    BinlogEventMap.insert(std::make_pair( SLAVE_EVENT ,0));
//    BinlogEventMap.insert(std::make_pair( CREATE_FILE_EVENT ,4));
    BinlogEventMap.insert(std::make_pair( APPEND_BLOCK_EVENT ,4));
//    BinlogEventMap.insert(std::make_pair( EXEC_LOAD_EVENT ,4));
    BinlogEventMap.insert(std::make_pair( DELETE_FILE_EVENT ,4));
//    BinlogEventMap.insert(std::make_pair( NEW_LOAD_EVENT ,18));
    BinlogEventMap.insert(std::make_pair( RAND_EVENT ,0));
    BinlogEventMap.insert(std::make_pair( USER_VAR_EVENT ,0));
    BinlogEventMap.insert(std::make_pair( FORMAT_DESCRIPTION_EVENT ,84));
    BinlogEventMap.insert(std::make_pair( XID_EVENT ,0));
    BinlogEventMap.insert(std::make_pair( BEGIN_LOAD_QUERY_EVENT ,4));
    BinlogEventMap.insert(std::make_pair( EXECUTE_LOAD_QUERY_EVENT ,26));
    BinlogEventMap.insert(std::make_pair( TABLE_MAP_EVENT ,8));
//    BinlogEventMap.insert(std::make_pair( DELETE_ROWS_EVENTv0 ,0));
//    BinlogEventMap.insert(std::make_pair( UPDATE_ROWS_EVENTv0 ,0));
//    BinlogEventMap.insert(std::make_pair( WRITE_ROWS_EVENTv0 ,0));
    BinlogEventMap.insert(std::make_pair( INCIDENT_EVENT ,2));
//    BinlogEventMap.insert(std::make_pair( HEARTBEAT_EVENT ,0));
//    BinlogEventMap.insert(std::make_pair( DELETE_ROWS_EVENTv2 ,10));
//    BinlogEventMap.insert(std::make_pair( UPDATE_ROWS_EVENTv2 ,10));
//    BinlogEventMap.insert(std::make_pair( WRITE_ROWS_EVENTv2 ,10));

    // binlog event type of event name
    BinlogEventIntMap.insert(std::make_pair(0, UNKNOWN_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(1, START_EVENT_V3 ));
    BinlogEventIntMap.insert(std::make_pair(2, QUERY_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(3, STOP_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(4, ROTATE_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(5, INTVAR_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(7, SLAVE_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(9, APPEND_BLOCK_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(11, DELETE_FILE_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(13, RAND_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(14, USER_VAR_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(15, FORMAT_DESCRIPTION_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(16, XID_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(17, BEGIN_LOAD_QUERY_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(18, EXECUTE_LOAD_QUERY_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(19, TABLE_MAP_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(23, WRITE_ROWS_EVENT_V1 ));
    BinlogEventIntMap.insert(std::make_pair(24, UPDATE_ROWS_EVENT_V1 ));
    BinlogEventIntMap.insert(std::make_pair(25, DELETE_ROWS_EVENT_V1 ));
    BinlogEventIntMap.insert(std::make_pair(26, INCIDENT_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(27, HEARTBEAT_LOG_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(28, IGNORABLE_LOG_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(29, ROWS_QUERY_LOG_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(30, WRITE_ROWS_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(31, UPDATE_ROWS_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(32, DELETE_ROWS_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(33, GTID_LOG_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(34, ANONYMOUS_GTID_LOG_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(35, PREVIOUS_GTIDS_LOG_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(36, TRANSACTION_CONTEXT_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(37, VIEW_CHANGE_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(38, XA_PREPARE_LOG_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(39, PARTIAL_UPDATE_ROWS_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(40, TRANSACTION_PAYLOAD_EVENT ));
    BinlogEventIntMap.insert(std::make_pair(41, HEARTBEAT_LOG_EVENT_V2 ));

    return 0;



}