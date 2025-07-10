//
// Created by 白杰 on 2024/12/29.
//

#include <cstdio>
#include <cstdlib>
#include <string>
#include "type.h"
#include <iostream>


using namespace std;



uint32_t getTypeStoreByteCount1(vector<tableConstructData>& tableStructVec, int i, vector<uint8_t >& group, uint32_t *lpOff) {
    std::vector<uint8_t> fieldData;
    fieldData.clear();
    string nextColumnAttalign;
    if ((i + 1) >= tableStructVec.size()) {
        nextColumnAttalign = "4";
    } else {
        if (tableStructVec[i + 1].columnAttalign == "c") {
            nextColumnAttalign = "1";
        } else if (tableStructVec[i + 1].columnAttalign == "s") {
            nextColumnAttalign = "2";
        } else if (tableStructVec[i + 1].columnAttalign == "i") {
            nextColumnAttalign = "4";
        } else if (tableStructVec[i + 1].columnAttalign == "d") {
            nextColumnAttalign = "8";
        }
    }
    printf("nextColumnAttalign: %s ", nextColumnAttalign.c_str());
    switch (stoi(tableStructVec[i].columnTypeId)) {
        case 19:
            // name
            typeEventFuncMap["name"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 21:
            // smallint
            typeEventFuncMap["smallint"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 20:
            // bigint
            typeEventFuncMap["bigint"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 23:
            // int
            typeEventFuncMap["int"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 26:
            // oid
            typeEventFuncMap["oid"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 1700:
            // numric
            // fieldData里就是全部的数据，具体解析这些数据
            printf("\n numeric_begin \n");
            typeEventFuncMap["numeric"](fieldData, group, lpOff, nextColumnAttalign);
            printf("\n numeric_end \n");
            break;
        case 700:
            // float
            typeEventFuncMap["float"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 701:
            // double
            typeEventFuncMap["double"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 18:
            // char
            printf("\n __char__ \n");
            typeEventFuncMap["char"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 1043:
            // varchar
            printf("\n __varchar__ \n");
            typeEventFuncMap["varchar"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 25:
            // text
            typeEventFuncMap["text"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 17:
            // bytea
            typeEventFuncMap["bytea"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 16:
            // bool
            typeEventFuncMap["bool"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 1082:
            // date
            typeEventFuncMap["date"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 1083:
            // time
            typeEventFuncMap["time"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 1114:
            // timestamp
            typeEventFuncMap["timestamp"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 1042:
            // bpchar
            // 小于127占一个字节 大于等于127的占4个字节
            // 这里进行计算，把长度，标志码什么的都计算出来再返回去
            // 取数据的第一个字节，判断是1B还是4B还是什么类型
            // 如果是1B，那么第一个字节就包含了长度，直接计算长度
            // 如果是4B，那么要取剩余的三个字节，再加上第一个字节，共4字节，包含了1位的标志位，1位标识是否压缩存储，剩余位为长度
//            return VARATT_IS_1B();
            // (length & (align - 1)) == 0 检查是否对齐
            printf("\n __bpchar__begin \n");
            typeEventFuncMap["bpchar"](fieldData, group, lpOff, nextColumnAttalign);
            printf("\n __bpchar__end \n");
            break;
        case 1184:
            // timestamp with time zone
            typeEventFuncMap["timestampWithTimeZone"](fieldData, group, lpOff, nextColumnAttalign);
            break;
        case 1186:
            // interval
            typeEventFuncMap["interval"](fieldData, group, lpOff, nextColumnAttalign);
        case 2950:
            // uuid
            typeEventFuncMap["uuid"](fieldData, group, lpOff, nextColumnAttalign);
        case 869:
            // inet
            typeEventFuncMap["inet"](fieldData, group, lpOff, nextColumnAttalign);
        case 650:
            // cidr
            typeEventFuncMap["cidr"](fieldData, group, lpOff, nextColumnAttalign);
        case 829:
            // macaddr
            typeEventFuncMap["macaddr"](fieldData, group, lpOff, nextColumnAttalign);
        case 114:
            // json
            typeEventFuncMap["json"](fieldData, group, lpOff, nextColumnAttalign);
        case 3802:
            // jsonb
            typeEventFuncMap["jsonb"](fieldData, group, lpOff, nextColumnAttalign);
        case 1033:
            // hstore
            typeEventFuncMap["hstore"](fieldData, group, lpOff, nextColumnAttalign);
        case 3904:
            // int4range
            typeEventFuncMap["int4range"](fieldData, group, lpOff, nextColumnAttalign);
        case 3906:
            // numrange
            typeEventFuncMap["numrange"](fieldData, group, lpOff, nextColumnAttalign);
        case 3908:
            // tsrange
            typeEventFuncMap["tsrange"](fieldData, group, lpOff, nextColumnAttalign);
        case 3910:
            // tstzrange
            typeEventFuncMap["tstzrange"](fieldData, group, lpOff, nextColumnAttalign);
        case 3912:
            // daterange
            typeEventFuncMap["daterange"](fieldData, group, lpOff, nextColumnAttalign);
        default:
            typeEventFuncMap["default"](fieldData, group, lpOff, nextColumnAttalign);
            return 0;
    }
}

//
//uint32_t getTypeStoreByteCount(vector<tableStructData>& tableStructVec, int i, vector<uint8_t >& group) {
//
//    std::vector<uint8_t> fieldData;
//    fieldData.clear();
//    switch (tableStructVec[i].columnTypeId) {
//        case 19:
//            // name
//            typeEventFuncMap["name"](fieldData, group);
//            break;
//        case 21:
//            // smallint
//            typeEventFuncMap["smallint"](fieldData, group);
//            break;
//        case 20:
//            // bigint
//            typeEventFuncMap["bigint"](fieldData, group);
//            break;
//        case 23:
//            // int
//            typeEventFuncMap["int"](fieldData, group);
//            break;
//        case 26:
//            // oid
//            typeEventFuncMap["oid"](fieldData, group);
//            break;
//        case 1700:
//            // numric
//            // fieldData里就是全部的数据，具体解析这些数据
//            printf("numeric_ \n");
//            typeEventFuncMap["numeric"](fieldData, group);
//        case 700:
//            // float
//            typeEventFuncMap["float"](fieldData, group);
//            break;
//        case 701:
//            // double
//            typeEventFuncMap["double"](fieldData, group);
//            break;
//        case 18:
//            // char
//            // 暂时不做解析，查询pg_attribute表中的atttypid = 18，都是系统表，没有普通表
//            typeEventFuncMap["char"](fieldData, group);
//        case 1043:
//            // varchar
//
//            typeEventFuncMap["varchar"](fieldData, group);
//        case 25:
//            // text
//
//            typeEventFuncMap["text"](fieldData, group);
//        case 17:
//            // bytea
//            typeEventFuncMap["bytea"](fieldData, group);
//        case 16:
//            // bool
//            fieldData.assign(group.begin(), group.begin() + 1);
//            group.erase(group.begin(), group.begin() + 1); // 移除已取出的字节
//            typeEventFuncMap["bool"](fieldData, group);
//            break;
//        case 1082:
//            // date
//            fieldData.assign(group.begin(), group.begin() + 4);
//            group.erase(group.begin(), group.begin() + 4); // 移除已取出的字节
//            typeEventFuncMap["date"](fieldData, group);
//            break;
//        case 1083:
//            // time
//            fieldData.assign(group.begin(), group.begin() + 8);
//            group.erase(group.begin(), group.begin() + 8); // 移除已取出的字节
//            typeEventFuncMap["time"](fieldData, group);
//            break;
//        case 1114:
//            // timestamp
//            fieldData.assign(group.begin(), group.begin() + 8);
//            group.erase(group.begin(), group.begin() + 8); // 移除已取出的字节
//            typeEventFuncMap["timestamp"](fieldData, group);
//            break;
//        case 1042:
//            // bpchar
//            // 小于127占一个字节 大于等于127的占4个字节
//            // 这里进行计算，把长度，标志码什么的都计算出来再返回去
//            // 取数据的第一个字节，判断是1B还是4B还是什么类型
//            // 如果是1B，那么第一个字节就包含了长度，直接计算长度
//            // 如果是4B，那么要取剩余的三个字节，再加上第一个字节，共4字节，包含了1位的标志位，1位标识是否压缩存储，剩余位为长度
////            return VARATT_IS_1B();
//            // (length & (align - 1)) == 0 检查是否对齐
//            typeEventFuncMap["bpchar"](fieldData, group);
//            break;
//        case 1184:
//            // timestamp with time zone
//            fieldData.assign(group.begin(), group.begin() + 8);
//            group.erase(group.begin(), group.begin() + 8); // 移除已取出的字节
//            typeEventFuncMap["timestampWithTimeZone"](fieldData, group);
//            break;
//        case 1186:
//            // interval
//            typeEventFuncMap["interval"](fieldData, group);
//        case 2950:
//            // uuid
//            typeEventFuncMap["uuid"](fieldData, group);
//        case 869:
//            // inet
//            typeEventFuncMap["inet"](fieldData, group);
//        case 650:
//            // cidr
//            typeEventFuncMap["cidr"](fieldData, group);
//        case 829:
//            // macaddr
//            typeEventFuncMap["macaddr"](fieldData, group);
//        case 114:
//            // json
//            typeEventFuncMap["json"](fieldData, group);
//        case 3802:
//            // jsonb
//            typeEventFuncMap["jsonb"](fieldData, group);
//        case 1033:
//            // hstore
//            typeEventFuncMap["hstore"](fieldData, group);
//        case 3904:
//            // int4range
//            typeEventFuncMap["int4range"](fieldData, group);
//        case 3906:
//            // numrange
//            typeEventFuncMap["numrange"](fieldData, group);
//        case 3908:
//            // tsrange
//            typeEventFuncMap["tsrange"](fieldData, group);
//        case 3910:
//            // tstzrange
//            typeEventFuncMap["tstzrange"](fieldData, group);
//        case 3912:
//            // daterange
//            typeEventFuncMap["daterange"](fieldData, group);
//        default:
//            typeEventFuncMap["default"](fieldData, group);
//            return 0;
//    }
//}

string getColumnTypeName(uint32_t typeID) {
    InitEventFuncMap();
    switch (typeID) {
        case 21:
            return "smallint";
        case 20:
            return "bigint";
        case 23:
            return "int";
        case 1700:
            printf("_numeric_ \n");
            return "numeric";
        case 700:
            return "float";
        case 701:
            return "double";
        case 18:
            return "char";
        case 1043:
            return "varchar";
        case 25:
            return "text";
        case 17:
            return "bytea";
        case 16:
            return "boolean";
        case 1082:
            return "date";
        case 1083:
            return "time";
        case 1114:
            return "timestamp";
        case 1042:
            return "bpchar";
        case 1184:
            return "timestamp with time zone";
        case 1186:
            return "interval";
        case 2950:
            return "uuid";
        case 869:
            return "inet";
        case 650:
            return "cidr";
        case 829:
            return "macaddr";
        case 114:
            return "json";
        case 3802:
            return "jsonb";
        case 1033:
            return "hstore";
        case 3904:
            return "int4range";
        case 3906:
            return "numrange";
        case 3908:
            return "tsrange";
        case 3910:
            return "tstzrange";
        case 3912:
            return "daterange";
        default:
            return "";
    }

};