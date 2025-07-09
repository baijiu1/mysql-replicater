//
// Created by 白杰 on 2024/12/14.
//

#ifndef MYSQL_REPLICATER_IBTREE_H
#define MYSQL_REPLICATER_IBTREE_H
#include <string>

using namespace std;

typedef struct IndexStateData {
    string indexName;
    string indexOID;
    int indexOpen(int oid, int lockMode);
    int indexTryOpen(int oid, int lockMode);
    int indexClose(int oid, int lockMode);
    int indexStates();
} TableStateData;

typedef IndexStateData *IndexState;


#endif //MYSQL_REPLICATER_IBTREE_H
